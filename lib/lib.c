#include <icc/icc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static inline bool
within_distance_hsv(
    color_hsv_t c1,
    color_hsv_t c2,
    int32_t distance);

static color_rgb_t
hsv_to_rgb(color_hsv_t hsv);

static color_hsv_t
rgb_to_hsv(color_rgb_t rgb);

static void
sort_by_hue(colors_t *colors);

/* TODO: Improve performance
 * TODO: Tweak parameters, try different averaging methods
 * TODO: Find better way to test color proximity
 */
int
get_dominant_colors(
    colors_t *colors,
    color_type_t out_type, colors_t *out)
{
    const int32_t scan_radius = 8;
    const uint32_t core_requirement = colors->count * 0.03;
    size_t *clusters = calloc(colors->count, sizeof(size_t));

    /* Sort the colors by hue to make processing more
     * efficient later. If color is not yet in HSV format,
     * convert it at the same time.
     */
    if (colors->type != ICC_COLOR_TYPE_HSV)
    {
        fprintf(
            stderr,
            "Converting colors from RGB to HSV...]\n"
        );

        colors->type = ICC_COLOR_TYPE_HSV;
        for (size_t i = 0; i < colors->count; ++i)
        {
            colors->colors[i].hsv =
                rgb_to_hsv(colors->colors[i].rgb);
        }
    }
    sort_by_hue(colors);

    /* We divide the colors into two different groups:
     * - colors that are a core part of the cluster; and
     * - colors that are not a core part.
     * We sort the array so, that the first part of the
     * array contains the core colors and the latter part
     * contains non-core colors. This helps with creating
     * the clusters later.
     */
    fprintf(
        stderr,
        "Separating %zu colors into cores...\n",
        colors->count
    );

    size_t cores_count = 0;
    size_t normals_count = 0;
    color_hsv_t *colors_separated =
        calloc(colors->count, sizeof(color_hsv_t));
    for (size_t i = 0; i < colors->count; ++i)
    {
        // if (i % 1000 == 0)
        // {
        //     fprintf(stderr, "Scanning color %zu...\n", i);
        // }

        color_hsv_t color = colors->colors[i].hsv;
        size_t colors_within_radius = 0;
        if (i != 0)
        {
            for (int j = i; j > 0; --j)
            {
                if (!within_distance_hsv(
                    color,
                    colors->colors[j-1].hsv,
                    scan_radius))
                {
                    break;
                }
                ++colors_within_radius;
            }
        }
        for (int j = i+1; j < colors->count; ++j)
        {
            if (!within_distance_hsv(
                color,
                colors->colors[j].hsv,
                scan_radius))
            {
                break;
            }
            ++colors_within_radius;
        }

        if (colors_within_radius >= core_requirement)
        {
            colors_separated[cores_count] = color;
            ++cores_count;
        }
        else
        {
            ++normals_count;
            size_t index = colors->count - normals_count;
            colors_separated[index] = color;
        }
    }

    fprintf(
        stderr,
        "Found %zu core colors and %zu non-core colors\n",
        cores_count, normals_count
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

        fprintf(
            stderr,
            "Creating cluster %zu...\n",
            cluster_count
        );

        /* We first check all the core colors and see if
         * any of them are close enough to the core colors
         * of the current cluster and if there are any, put
         * them into the cluster.
         */
        for (int k = start; k < colors_processed; ++k)
        {
            for (size_t i = colors_processed; i < cores_count; ++i)
            {
                if (within_distance_hsv(
                    colors_separated[k],
                    colors_separated[i],
                    scan_radius))
                {
                    if (i != colors_processed)
                    {
                        color_hsv_t temp = colors_separated[i];
                        colors_separated[i] = colors_separated[colors_processed];
                        colors_separated[colors_processed] = temp;
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
        for (size_t i = cores_count; i < colors->count; ++i)
        {
            for (size_t j = start; j < colors_processed; ++j)
            {
                if (within_distance_hsv(
                        colors_separated[i],
                        colors_separated[j],
                        scan_radius))
                {
                    size_t index = colors_processed + colors_in_cluster;
                    if (i != cores_count)
                    {
                        color_hsv_t temp = colors_separated[i];
                        colors_separated[i] = colors_separated[cores_count];
                        colors_separated[cores_count] = temp;
                    }
                    color_hsv_t temp = colors_separated[cores_count];
                    colors_separated[cores_count] = colors_separated[index];
                    colors_separated[index] = temp;
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
        .colors = calloc(cluster_count, sizeof(color_t)),
        .type = out_type
    };
    size_t *dominant_counts = calloc(cluster_count, sizeof(size_t));
    for (size_t i = 0; i < colors_processed;)
    {
        // Calculate average and count the number of colors.
        size_t current_cluster = clusters[i];
        size_t avg_h = colors_separated[i].h;
        size_t avg_s = colors_separated[i].s;
        size_t avg_v = colors_separated[i].v;
        size_t colors_count = 1;
        ++i;

        fprintf(
            stderr,
            "Calculating average value of cluster %zu...\n",
            current_cluster
        );

        while (i < colors_processed)
        {
            if (clusters[i] != current_cluster) break;
            color_hsv_t color = colors_separated[i];
            avg_h += color.h;
            avg_s += color.s;
            avg_v += color.v;
            ++colors_count;
            ++i;
        }

        color_t avg = {
            .hsv = {
                .h = avg_h / colors_count,
                .s = avg_s / colors_count,
                .v = avg_v / colors_count
            }
        };
        if (out_type == ICC_COLOR_TYPE_RGB)
        {
            avg.rgb = hsv_to_rgb(avg.hsv);
        }

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
    free(colors_separated);
    free(clusters);

    *out = dominants;

    return 0;
}

static void
sort_by_hue_impl(
    color_t *colors,
    size_t start,
    size_t length);

static void
sort_by_hue(colors_t *colors)
{
    fprintf(stderr, "Sorting colors by hue...\n");
    sort_by_hue_impl(colors->colors, 0, colors->count);
}

static void
sort_by_hue_impl(
    color_t *colors,
    size_t start,
    size_t length)
{
    if (length <= 1) return;

    size_t half = length / 2;
    size_t mid = start + half;
    sort_by_hue_impl(colors, start, half);
    sort_by_hue_impl(colors, mid, length - half);

    size_t l = start;
    size_t r = mid;
    size_t end = start + length;

    while (l < mid && r < end)
    {
        color_t rv = colors[r];
        if (colors[l].hsv.h > rv.hsv.h)
        {
            for (size_t i = r; i > l; --i)
            {
                colors[i] = colors[i-1];
            }
            colors[l] = rv;
            ++mid;
            ++r;
        }
        ++l;
    }
}

static color_hsv_t
rgb_to_hsv(color_rgb_t rgb)
{
    color_hsv_t hsv = {0};

    uint16_t values[3] = {0};
    if (rgb.r >= rgb.g && rgb.r >= rgb.b)
    {
        if (rgb.b > rgb.g)
        {
            hsv.h = 300;
            values[0] = rgb.r;
            values[1] = rgb.b;
            values[2] = rgb.g;
        }
        else
        {
            hsv.h = 0;
            values[0] = rgb.r;
            values[1] = rgb.g;
            values[2] = rgb.b;
        }
    }
    else if (rgb.g >= rgb.r && rgb.g >= rgb.b)
    {
        if (rgb.r > rgb.b)
        {
            hsv.h = 60;
            values[0] = rgb.g;
            values[1] = rgb.r;
            values[2] = rgb.b;
        }
        else
        {
            hsv.h = 120;
            values[0] = rgb.g;
            values[1] = rgb.b;
            values[2] = rgb.r;
        }
    }
    else
    {
        if (rgb.g > rgb.r)
        {
            hsv.h = 180;
            values[0] = rgb.b;
            values[1] = rgb.g;
            values[2] = rgb.r;
        }
        else
        {
            hsv.h = 240;
            values[0] = rgb.b;
            values[1] = rgb.r;
            values[2] = rgb.g;
        }
    }

    hsv.v = (100 * values[0]) / 255;
    if (values[1] == values[2])
    {
        hsv.s = (100 - (100 * values[1]) / 255)
            * (values[0] != values[1]);
    }
    else
    {
        uint16_t actual_2 = (values[2] * 255) / values[0];
        hsv.s = 100 - (100 * (uint16_t)actual_2) / 255;

        uint16_t actual_1 =
            ((values[1] * 100 * 255) / values[0]
                - (255 * (100 - (uint16_t)hsv.s))
            ) / hsv.s;
        hsv.h += (actual_1 * 60) / 255;

        // fprintf(
        //     stderr,
        //     "rgb(%hu,%hu,%hu) -> hsv(%hu,%hhu,%hhu)\n",
        //     values[0], values[1], values[2],
        //     hsv.h, hsv.s, hsv.v
        // );
    }

    return hsv;
}

static color_rgb_t
hsv_to_rgb(color_hsv_t hsv)
{
    color_rgb_t rgb = {0};

    hsv.h %= 360;
    uint8_t *values[3] = {0};
    if (hsv.h < 60)
    {
        values[0] = &rgb.r;
        values[1] = &rgb.g;
        values[2] = &rgb.b;
    }
    else if (hsv.h < 120)
    {
        values[0] = &rgb.g;
        values[1] = &rgb.r;
        values[2] = &rgb.b;
    }
    else if (hsv.h < 180)
    {
        values[0] = &rgb.g;
        values[1] = &rgb.b;
        values[2] = &rgb.r;
    }
    else if (hsv.h < 240)
    {
        values[0] = &rgb.b;
        values[1] = &rgb.g;
        values[2] = &rgb.r;
    }
    else if (hsv.h < 300)
    {
        values[0] = &rgb.b;
        values[1] = &rgb.r;
        values[2] = &rgb.g;
    }
    else if (hsv.h < 360)
    {
        values[0] = &rgb.r;
        values[1] = &rgb.b;
        values[2] = &rgb.g;
    }

    hsv.h = ((hsv.h / 60) % 2 == 0) * (hsv.h % 60)
        + ((hsv.h / 60) % 2 != 0) * (60 - hsv.h % 60);
    uint16_t sm = 100 - (uint16_t)hsv.s;
    uint16_t v1 = (255 * (uint16_t)hsv.h) / 60;
    uint16_t v2 = (255 * sm) / 100;

    *values[0] = (255 * (uint16_t)hsv.v) / 100;
    *values[1] = (((v1 * sm) / 100) * (uint16_t)hsv.v) / 100;
    *values[2] = (v2 * (uint16_t)hsv.v) / 100;

    return rgb;
}

static inline bool
within_distance_hsv(
    color_hsv_t c1,
    color_hsv_t c2,
    int32_t distance)
{
    int32_t dh = (int32_t)c1.h - (int32_t)c2.h;
    // int32_t ds = (int32_t)c1.s - (int32_t)c2.s;
    // int32_t dv = (int32_t)c1.v - (int32_t)c2.v;
    // return dh*dh + ds*ds + dv*dv <= distance * distance;
    return dh*dh <= distance * distance;
}
