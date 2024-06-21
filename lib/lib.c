#include <icc/icc.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static bool
within_distance(color_t c1, color_t c2, float radius);

/* TODO: Improve performance
 * TODO: Tweak parameters, try different averaging methods
 */
int get_dominant_colors(colors_t colors, colors_t *out)
{
    const float scan_radius = 10.f;
    const unsigned int core_requirement = 10;
    size_t *clusters = calloc(colors.count, sizeof(size_t));

    /* We divide the colors into two different groups:
     * - colors that are a core part of the cluster; and
     * - colors that are not a core part.
     * We sort the array so, that the first part of the
     * array contains the core colors and the latter part
     * contains non-core colors. This helps with creating
     * the clusters later.
     */
    fprintf(stderr, "Separating %zu colors into cores...\n", colors.count);

    size_t cores_count = 0;
    for (size_t i = 0; i < colors.count; ++i)
    {
        if (i % 1000 == 0)
        {
            fprintf(stderr, "Scanning color %zu...\n", i);
        }

        size_t colors_within_radius = 0;
        for (size_t j = 0; j < colors.count; ++j)
        {
            colors_within_radius += (j != i)
                && within_distance(
                    colors.colors[i],
                    colors.colors[j],
                    scan_radius
                );
        }

        if (colors_within_radius >= core_requirement)
        {
            if (cores_count != i)
            {
                color_t temp = colors.colors[i];
                colors.colors[i] = colors.colors[cores_count];
                colors.colors[cores_count] = temp;
            }
            ++cores_count;
        }
    }

    fprintf(
        stderr,
        "Found %zu core colors and %zu non-core colors\n",
        cores_count, colors.count - cores_count
    );

    size_t cluster_count = 0;
    size_t colors_processed = 0;
    while (colors_processed < cores_count)
    {
        size_t start = colors_processed;

        /* Picks the first core color that has yet to be
         * processed and use it as the first data point
         * for a cluster.
         */
        clusters[colors_processed] = cluster_count;
        ++colors_processed;

        fprintf(stderr, "Creating cluster %zu...\n", cluster_count);

        /* We first check all the core colors and see if
         * any of them are close enough to the core colors
         * of the current cluster and if there are any, put
         * them into the cluster.
         */
        for (int k = start; k < colors_processed; ++k)
        {
            for (size_t i = colors_processed; i < cores_count; ++i)
            {
                if (within_distance(
                    colors.colors[k],
                    colors.colors[i],
                    scan_radius))
                {
                    if (i != colors_processed)
                    {
                        color_t temp = colors.colors[i];
                        colors.colors[i] = colors.colors[colors_processed];
                        colors.colors[colors_processed] = temp;
                    }
                    clusters[colors_processed] = cluster_count;
                    ++colors_processed;
                }
            }
        }

        fprintf(
            stderr,
            "Found %zu core colors in cluster %zu\n",
            colors_processed - start, cluster_count
        );

        /* We then check if any of the non-core colors are
         * close enough to the current cluster and if there
         * are any, put them into the cluster.
         */
        size_t colors_in_cluster = 0;
        for (size_t i = cores_count; i < colors.count; ++i)
        {
            for (size_t j = start; j < colors_processed; ++j)
            {
                if (within_distance(
                    colors.colors[i],
                    colors.colors[j],
                    scan_radius))
                {
                    size_t index = colors_processed + colors_in_cluster;
                    if (i != cores_count)
                    {
                        color_t temp = colors.colors[i];
                        colors.colors[i] = colors.colors[cores_count];
                        colors.colors[cores_count] = temp;
                    }
                    color_t temp = colors.colors[cores_count];
                    colors.colors[cores_count] = colors.colors[index];
                    colors.colors[index] = temp;
                    clusters[index] = cluster_count;
                    colors_in_cluster += 1;
                    cores_count += 1;
                    break;
                }
            }
        }
        colors_processed += colors_in_cluster;

        fprintf(
            stderr,
            "Found %zu non-core colors in cluster %zu\n",
            colors_in_cluster, cluster_count
        );

        ++cluster_count;
    }

    /* We now take the average value of each cluster and
     * sort them by the amount of colors in the clusters
     * from largest to smallest.
     */
    colors_t dominants = {
        .count = 0,
        .colors = calloc(cluster_count, sizeof(color_t))
    };
    size_t *dominant_counts = calloc(cluster_count, sizeof(size_t));
    for (size_t i = 0; i < colors_processed;)
    {
        // Calculate average and count the number of colors.
        size_t current_cluster = clusters[i];
        size_t avg_r = colors.colors[i].r;
        size_t avg_g = colors.colors[i].g;
        size_t avg_b = colors.colors[i].b;
        size_t colors_count = 1;
        ++i;

        fprintf(
            stderr,
            "Calculating average value of cluster %zu...\n",
            cluster_count
        );

        while (i < colors_processed)
        {
            if (clusters[i] != current_cluster) break;
            color_t color = colors.colors[i];
            avg_r += color.r;
            avg_g += color.g;
            avg_b += color.b;
            ++colors_count;
            ++i;
        }

        color_t avg = {
            .r = avg_r / colors_count,
            .g = avg_g / colors_count,
            .b = avg_b / colors_count
        };

        // Insert the average value to the right position.
        size_t insert_idx = dominants.count;
        if (dominants.count != 0)
        {
            for (size_t i = 0; i < dominants.count; ++i)
            {
                if (colors_count > dominant_counts[i])
                {
                    insert_idx = i;
                    break;
                }
            }

            for (size_t i = dominants.count; i > insert_idx; --i)
            {
                dominants.colors[i] = dominants.colors[i-1];
                dominant_counts[i] = dominant_counts[i-1];
            }
        }

        dominants.colors[insert_idx] = avg;
        dominant_counts[insert_idx] = colors_count;
        ++dominants.count;
    }

    free(dominant_counts);
    free(clusters);

    *out = dominants;

    return 0;
}

static bool
within_distance(color_t c1, color_t c2, float distance)
{
    float x = (float)c2.r - (float)c1.r;
    float y = (float)c2.g - (float)c1.g;
    float z = (float)c2.b - (float)c1.b;

    return sqrtf(x*x + y*y + z*z) <= distance;
}
