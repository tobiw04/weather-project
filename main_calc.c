#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATION_COUNT 13
#define MAX_RECORDS 600000

// Test
#define WESTERN_MOST_STATION 0

// define the path to the csv files
#define path_Gatow_1 "output/02_Gatow-1_2022_out.csv"
#define path_BotanischerGarten_1 "output/04_Botanischer-Garten-1_2022_out.csv"
#define path_Marzahn_1 "output/06_Marzahn-1_2022_out.csv"
#define path_Tegel_1 "output/07_Tegel-Forstamt-1_2022_out.csv"
#define path_Wannsee_1 "output/12_Wannsee-1_2022_out.csv"
#define path_Planertarium_1 "output/13_Planetarium-1_2022_out.csv"
#define path_Pichelsdorf_1 "output/14_Pichelsdorf-1_2022_out.csv"
#define path_BotanischerGarten_2 "output/15_Botanischer-Garten-2_2022_out.csv"
#define path_Funkturm_1 "output/16_Funkturm-1_2022_out.csv"
#define path_Tegel_2 "output/17_Tegeler-See-1_2022_out.csv"
#define path_FB_1 "output/18_FB-Messwiese-2_2022_out.csv"
#define path_Mueggelsee_1 "output/19_Mueggelsee-1_2022_out.csv"
// #define path_FB_2 "output/20_FB-Messturm-2_2022_out.csv"
#define path_Moabit_1 "output/21_Moabit-1_2022_out.csv"

char *paths[STATION_COUNT] = {
    path_Gatow_1, path_BotanischerGarten_1, path_Marzahn_1, path_Tegel_1,
    path_Wannsee_1, path_Planertarium_1, path_Pichelsdorf_1, path_BotanischerGarten_2,
    path_Funkturm_1, path_Tegel_2, path_FB_1, path_Mueggelsee_1,
    /*path_FB_2,*/ path_Moabit_1};

// Datatyp for the important Data
typedef struct
{
    int y, m, d, h, min;
    float temp;
} WeatherRecord;

// Datatyp that saves the data and the "id"
typedef struct
{
    WeatherRecord *records;
    int count;
} WeatherData;

// Datatyp to know the neighbors
typedef struct
{
    int north, east, south, west;
} Neighbors;

// all neighbors
// NOTE: STATION_COUNT == 13 means valid indices are 0..12
Neighbors neighbors[STATION_COUNT] = {
    {6, -1, 4, -1},  // 0
    {8, 10, -1, 4},  // 1
    {-1, -1, 11, 5}, // 2
    {-1, -1, 9, -1}, // 3
    {0, 1, -1, -1},  // 4
    {-1, 2, -1, -1}, // 5  (fixed: removed 13 -> -1)
    {9, 8, 4, 0},    // 6
    {8, 10, -1, 4},  // 7
    {9, -1, 10, 6},  // 8
    {3, -1, -1, 8},  // 9
    {8, -1, -1, 7},  // 10
    {2, -1, -1, -1}, // 11
    {3, 5, 10, 8}    // 12
};

// read from the csv file
WeatherData load_csv(const char *path)
{
    printf("loading file '%s'...\n", path);
    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "Critical Error: File '%s' couldn't be opened!\n", path);
        return (WeatherData){NULL, 0};
    }

    WeatherRecord *rec = malloc(sizeof(WeatherRecord) * MAX_RECORDS); // rec[i] is the same as *(rec + i) - can be used as array later
    if (!rec)
    {
        perror("Memory allocation failed");
        fclose(f);
        return (WeatherData){NULL, 0};
    }

    char line[2048];
    int count = 0;

    // Read lines; skip header/comment lines starting with '#' or empty lines
    while (fgets(line, sizeof(line), f))
    {
        // skip comments or empty lines
        char *p = line;
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\n' || *p == '\0' || *p == '#')
            continue;

        // Try parsing with both ';' and ',' delimiters.
        WeatherRecord r;
        int fields = 0;

        // Try semicolon-separated first
        fields = sscanf(line, "%d;%d;%d;%d;%d;%f", &r.y, &r.m, &r.d, &r.h, &r.min, &r.temp);
        if (fields != 6)
        {
            // Try comma-separated
            fields = sscanf(line, "%d,%d,%d,%d,%d,%f", &r.y, &r.m, &r.d, &r.h, &r.min, &r.temp);
        }
        // If still not parsed, try more tolerant parsing (spaces)
        if (fields != 6)
        {
            fields = sscanf(line, "%d %d %d %d %d %f", &r.y, &r.m, &r.d, &r.h, &r.min, &r.temp);
        }

        if (fields == 6)
        {
            if (count < MAX_RECORDS)
            {
                rec[count] = r;
                count++;
            }
            else
            {
                fprintf(stderr, "Warning: reached MAX_RECORDS (%d) while reading '%s'.\n", MAX_RECORDS, path);
                break;
            }
        }
        else
        {
            // If a single non-header line fails to parse, print a debug message and continue.
            // This helps if the CSV contains malformed lines.
            fprintf(stderr, "Warning: could not parse line %d in '%s': %s", count + 1, path, line);
            continue;
        }
    }

    fclose(f);

    if (count == 0)
    {
        fprintf(stderr, "Error: File %s does not contain any Data. count=0\n", path);
        free(rec);
        return (WeatherData){NULL, 0};
    }

    // resize Memory allocation to actual count
    WeatherRecord *final_rec = realloc(rec, sizeof(WeatherRecord) * count);
    if (final_rec == NULL)
    {
        fprintf(stderr, "WARNUNG: Reallocation failed, using original buffer.\n");
        final_rec = rec;
    }

    return (WeatherData){final_rec, count};
}

// calculate the local temps
void compute_local(float *local, WeatherData *d)
{
    if (d->count > 0) // d??
    {
        local[0] = 0.0f; // start value

        // difference between old value and new value
        for (int i = 1; i < d->count; i++)
        {
            local[i] = d->records[i].temp - d->records[i - 1].temp;
        }
    }
}

// calculate the temps between the neighbours
void compute_final(float *final, float *local[], WeatherData data[], int station_id, Neighbors neigh[])
{
    int my_len = data[station_id].count;
    Neighbors n = neigh[station_id];

    for (int t = 0; t < my_len; t++)
    {
        float sum = 0.0f;
        int cnt = 0;

        // own station
        sum += local[station_id][t];
        cnt++;

        // north
        if (n.north >= 0 && t < data[n.north].count)
        {
            sum += local[n.north][t];
            cnt++;
        }

        // south
        if (n.south >= 0 && t < data[n.south].count)
        {
            sum += local[n.south][t];
            cnt++;
        }

        // east
        if (n.east >= 0 && t < data[n.east].count)
        {
            sum += local[n.east][t];
            cnt++;
        }

        // west
        if (n.west >= 0 && t < data[n.west].count)
        {
            sum += local[n.west][t];
            cnt++;
        }

        final[t] = (cnt > 0) ? (sum / cnt) : 0.0f;
    }
}

// write the data in a csv file
void write_csv(const char *name, float *final, WeatherData *src)
{
    // build a csv file
    char outname[256];
    sprintf(outname, "output_station_%s.csv", name);

    FILE *f = fopen(outname, "w");
    if (!f)
    {
        perror("Ausgabedatei Fehler");
        return;
    }

    // first line of the csv file
    fprintf(f, "year,month,day,hour,minute,temp_change\n");
    fprintf(f, "# begin of data\n");

    // write the data in the file
    for (int i = 0; i < src->count; i++)
    {
        WeatherRecord *r = &src->records[i];
        fprintf(f, "%d,%d,%d,%d,%d,%.3f\n",
                r->y, r->m, r->d, r->h, r->min, final[i]);
    }

    // close the csv file
    fclose(f);
}


//------------------------------------------------------------------
int main()
{
    WeatherData data[STATION_COUNT];
    float *local[STATION_COUNT];
    float *final[STATION_COUNT];
    int rec_count = -1; // Should check consistency

    // initialize pointers to NULL to make cleanup safe
    for (int i = 0; i < STATION_COUNT; i++)
    {
        data[i].records = NULL;
        data[i].count = 0;
        local[i] = NULL;
        final[i] = NULL;
    }

    printf("read CSV file...\n");

    for (int i = 0; i < STATION_COUNT; i++)
    {
        // print which station is being processed
        printf("Station %d are being read: %s\n", i, paths[i]);

        data[i] = load_csv(paths[i]);

        //  CHECK: dataset must be valid 
        if (data[i].count == 0 || data[i].records == NULL)
        {
            fprintf(stderr, "critical Error: Station %d couldn't be read or it is empty. abort.\n", i);

            // free Memory allocation
            for (int j = 0; j < STATION_COUNT; j++)
            {
                if (data[j].records)
                    free(data[j].records);
                if (local[j])
                    free(local[j]);
                if (final[j])
                    free(final[j]);
            }
            return 1; // end program
        }

        //  Consistency check of number of measurements
        if (rec_count == -1)
        {
            printf("Consistency check successful\n");
            rec_count = data[i].count;
        }

        // memory allocation, because data[i].count > 0
        local[i] = malloc(sizeof(float) * data[i].count);
        final[i] = malloc(sizeof(float) * data[i].count);

        if (!local[i] || !final[i])
        {
            perror("memory allocation failed for local/final values");
            // cleanup
            for (int j = 0; j <= i; j++)
            {
                if (data[j].records)
                    free(data[j].records);
                if (local[j])
                    free(local[j]);
                if (final[j])
                    free(final[j]);
            }
            return 1;
        }
    }

    printf("All data successfully loaded (%d measurement points per station expected).\n", rec_count);

    // local temp calculation
    for (int i = 0; i < STATION_COUNT; i++)
    {
        compute_local(local[i], &data[i]);
    }

    // calculate temps of all
    for (int i = 0; i < STATION_COUNT; i++)
    {
        compute_final(final[i], local, data, i, neighbors);
    }

    // print results
    printf("print results...\n");
    for (int i = 0; i < STATION_COUNT; i++)
    {
        char buf[8];
        sprintf(buf, "%d", i);
        write_csv(buf, final[i], &data[i]);
    }
    printf("finished printing results\n");

    // free memory allocation
    for (int i = 0; i < STATION_COUNT; i++)
    {
        if (data[i].records)
            free(data[i].records);
        if (local[i])
            free(local[i]);
        if (final[i])
            free(final[i]);
    }

    printf("memory has been freed. Simulation finished.\n");
    return 0;
}
