#include "json_parser.h"
#include <stdio.h>
#include <string.h>

#define MAX_PRESET_TEMPERATURE_COUNT 8

const char *default_preset_array =
    "[{\"id\": 1, \"name\": \"Comfort\", \"heating\": 22.0, \"cooling\": 24.5}, "
    "{\"id\": 2, \"name\": \"Sleep\", \"heating\": 20.0, \"cooling\": 25.5}, "
    "{\"id\": 3, \"name\": \"Economy\", \"heating\": 26.5, \"cooling\": 18.0}]";

int main(void)
{
    jparse_ctx_t jctx;

    if (json_parse_start(&jctx, default_preset_array,
                         strlen(default_preset_array)) != 0)
    {
        printf("Failed to parse preset temperature JSON\n");
        return 0;
    }

    int count = 0;

    /* 特殊处理：假定 root 本身就是 array，直接按 index 试探 */
    for (int i = 0; i < MAX_PRESET_TEMPERATURE_COUNT; ++i)
    {
        if (json_arr_get_object(&jctx, i) != 0)
        {
            /* 越界或 root 不是 array，结束 */
            break;
        }

        int id = 0;
        char name[32] = {0};
        float heating = 0.0f;
        float cooling = 0.0f;

        json_obj_get_int(&jctx, "id", &id);
        json_obj_get_string(&jctx, "name", name, sizeof(name));
        json_obj_get_float(&jctx, "heating", &heating);
        json_obj_get_float(&jctx, "cooling", &cooling);

        printf("Parsed preset %d: id=%d, name=%s, heating=%.1f, cooling=%.1f\n",
               i, id, name, heating, cooling);

        json_arr_leave_object(&jctx);
        count++;
    }

    if (count == 0)
    {
        printf("No preset temperatures found in JSON\n");
    }
    else
    {
        printf("Found %d preset temperatures\n", count);
    }

    json_parse_end(&jctx);
    return 0;
}
