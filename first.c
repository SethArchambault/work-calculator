#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define assert(expr) if(!(expr)) { printf("\nline %d: %s\n", __LINE__, #expr); __builtin_trap();} 


int is_break(char value) {
    if (value == ' ' ||
        value == '\n') {
        return 1;
    } else {
        return 0;
    }
}

int is_number(char value) {
    int d = (int)value;
    if(d >= 48 && d <= 57) {
        return 1;
    } else {
        return 0;
    }
}

int is_char(char value) {
    int d = (int)value;
    if(d >= 97 && d <= 122) {
        return 1;
    } else {
        return 0;
    }
}



int parse_int(char * c, int * i) {
    // @Compress: into function
    const int timer_end_max = 5;
    char timer_end_char[timer_end_max]; 
    int timer_end_index = 0;
    //printf("\n-- found dash -\n");
    for(;is_number(c[*i]) ;++*i) {
        timer_end_char[timer_end_index] = (int)c[*i];
        ++timer_end_index;
        assert(timer_end_index <= timer_end_max); // make sure we don't segfault here..
    }
    timer_end_char[timer_end_index] = '\0';

    return atoi(timer_end_char);
}


#if 1
int main(int argc, char ** argv) {
    char variable_array[100][40];
    float budget_array[100];
    float spent_array[100];
    int variable_count = 0;
    assert(argc == 2);
    char *filename = argv[1];
    {
        int f_len;
        char * c;
        {
            FILE * f = fopen(filename, "r");
            assert(f);
            fseek(f, 0, SEEK_END);
            f_len = ftell(f);
            fseek(f, 0, SEEK_SET);
            c = malloc(f_len);
            fread(c, f_len, 1, f);
            fclose(f);
        }

        //
        // Step 1: Transform data into a single array of pages + times
        //         And pull variables from file
        //
        int mode = 0;
        char * reformatted_data = malloc(f_len + f_len * 0.5);
        for (int i = 0; i < f_len; ++i) {
            for (;c[i] == ' ';++i); // skip space
            if (mode == 0) {
                if (c[i] == '/') { // skip comment
                    for (;c[i] != '\n';++i) {
                        sprintf(reformatted_data, "%s%c", reformatted_data, c[i]);
                    }
                    strcat(reformatted_data, "\n");
                    continue;
                }
                if (c[i] == ':') { // assign variable
                    ++i;
                    char variable_name_len = 0;
                    // get variable name
                    for (;c[i] != ' '; ++i) {
                        variable_array[variable_count][variable_name_len] = c[i];
                        ++variable_name_len;
                    }
                    variable_array[variable_count][variable_name_len] = '\0';


                    for (;c[i] == ' ';++i); // skip whitespace
                    
                    char variable_value[50];
                    char variable_value_len = 0;
                    // get variable value
                    for (;!is_break(c[i]); ++i) {
                        variable_value[variable_value_len] = c[i];
                        ++variable_value_len;
                    }
                    variable_value[variable_value_len] = '\0';
                    
                    sprintf(reformatted_data, "%s:%s %s\n", 
                            reformatted_data, variable_array[variable_count], variable_value);

                    // probably search to make sure this doesn't already exist
                    budget_array[variable_count]    = atof(variable_value);
                    spent_array[variable_count]     = 0;
                    ++variable_count;

                    for (;c[i] != '\n';++i);
                    continue;
                }
                mode = 1;

            }

            if (mode == 1) {
                if (c[i] == '\n') {
                    mode = 0;
                    continue;
                }
                if (!is_char(c[i])) {
                    printf("Error: '");
                    for (;c[i] != '\n';++i) { // print rest of line 
                        printf("%c", c[i]);
                    }
                    printf("'");
                }
                assert(is_char(c[i])); 

                
                // parse the first word
                char variable_name[40];
                int variable_name_len = 0;
                for (;c[i] != ' '; ++i) {
                    variable_name[variable_name_len] = c[i];
                    ++variable_name_len;
                }
                variable_name[variable_name_len] = '\0';

                for (;c[i] == ' ';++i); // skip space

                // parse the value
                int timer_start = parse_int(c, &i);
                int timer_end   = 0;
                
                if (c[i] == '-') {
                    ++i;
                    timer_end = parse_int(c, &i);
                } else {
                    timer_end = timer_start;
                }
                //printf("timer_start %d\n", timer_start);
                //printf("timer_end %d\n", timer_end);

                int timer_dif = timer_end - timer_start;
                //printf("timer_dif %d\n", timer_dif);
                float timer_hours = (float)timer_dif / 60;
                //printf("timer_hours %f\n", timer_hours);

                // subtract the value in the dictionary
                for (int i = 0; i < variable_count;++i) {
                    if(strcmp(variable_array[i], variable_name) == 0 ) {
                        spent_array[i] += timer_hours;
                    }
                }


                for (;c[i] != '\n';++i); // fastforaard
                mode = 0;
                continue;

            } // mode
        } // for
        printf("\n\n");
        float total_budget = 0;
        float total_spent = 0;
        float total_fudge_factor = 0;
        int counted_in_fudge_factor = 0;
        for (int i = 0; i < variable_count;++i) {
            total_budget += budget_array[i];
            total_spent += spent_array[i];
            float net = budget_array[i] - spent_array[i];

            if (spent_array[i] == 0) {
                printf("\033[34m"); // blue
            }
            if (net < 0) {
                printf("\033[41m"); // red 
            }
            printf("%20s %3.1f - %3.1f = %5.1f", variable_array[i], budget_array[i], spent_array[i], net);
            if (spent_array[i] > 0) {
                float fudge_factor = spent_array[i] / budget_array[i];
                ++counted_in_fudge_factor;
                total_fudge_factor += fudge_factor;
                printf("  %5.1fx", fudge_factor);
            }
            printf("\033[0m"); // normal
            printf("\n");
        }
        printf("\n");
        printf("Total Budgeted  %5.2f\n", total_budget);
        printf("Total Spent     %5.2f\n", total_spent);
        printf("Total Remaining %5.2f\n", total_budget - total_spent);
        printf("Avg Fudge Factor %5.2f\n", total_fudge_factor / counted_in_fudge_factor);
        printf("\n");
        printf("Current Pay Per Hour $%.2f\n", 2500 / total_spent);
        printf("Projected Pay Per Hour $%.2f\n", 2500 / total_budget);

    } // scope

    return 0;
}
#endif
