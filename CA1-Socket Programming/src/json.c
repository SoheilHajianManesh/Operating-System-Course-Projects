#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "json.h"



char* readFile(){    
    int fd = open("recipes.json", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file information");
        close(fd);
    }

    char* json_data = (char*)malloc(file_stat.st_size + 1);
    if (!json_data) {
        perror("Memory allocation failed");
        close(fd);
    }

    // Read the file using read
    ssize_t read_size = read(fd, json_data, file_stat.st_size);
    close(fd);

    if (read_size == -1) {
        perror("Failed to read file");
        free(json_data);
    }

    json_data[read_size] = '\0';

    return json_data;
}
// Function to parse JSON data and populate your structs
void extractFoodsFromJson(const char* json_data, Foods* foods) {
    foods->numOfFoods = 0;
    int arraySize = 10;
    foods->food = (Food*)malloc(arraySize * sizeof(Food));

    const char* ptr = json_data;

    while (1) {
        const char* foodStart = strchr(ptr, '"');
        if (foodStart == NULL) {
            break;
        }

        const char* foodEnd = strchr(foodStart + 1, '"');
        if (foodEnd == NULL) {
            break;
        }

        int foodNameLength = foodEnd - foodStart - 1;
        if (foodNameLength >= NAME_SIZE) {
            foodNameLength = NAME_SIZE - 1;
        }

        strncpy(foods->food[foods->numOfFoods].name, foodStart + 1, foodNameLength);
        foods->food[foods->numOfFoods].name[foodNameLength] = '\0';

        const char* ingStart = strchr(foodEnd, '{');
        if (ingStart == NULL) {
            break;
        }

        const char* ingEnd = strchr(ingStart, '}');
        if (ingEnd == NULL) {
            break;
        }

        IngredientsArray* ings = &foods->food[foods->numOfFoods].ings;
        ings->numOfIngs = 0;
        ings->arr = (Ingredient*)malloc(arraySize * sizeof(Ingredient));

        const char* ingPtr = ingStart;

        while (1) {
            const char* ingNameStart = strchr(ingPtr, '"');
            if (ingNameStart == NULL || ingNameStart > ingEnd) {
                break;
            }

            const char* ingNameEnd = strchr(ingNameStart + 1, '"');
            if (ingNameEnd == NULL || ingNameEnd > ingEnd) {
                break;
            }

            int ingNameLength = ingNameEnd - ingNameStart - 1;
            if (ingNameLength >= NAME_SIZE) {
                ingNameLength = NAME_SIZE - 1;
            }

            strncpy(ings->arr[ings->numOfIngs].name, ingNameStart + 1, ingNameLength);
            ings->arr[ings->numOfIngs].name[ingNameLength] = '\0';

            const char* colon = strchr(ingNameEnd, ':');
            if (colon == NULL || colon > ingEnd) {
                break;
            }

            const char* comma = strchr(colon, ',');
            if (comma == NULL || comma > ingEnd) {
                comma = ingEnd;
            }

            ings->arr[ings->numOfIngs].amount = atoi(colon + 1);

            ingPtr = comma + 1;
            ings->numOfIngs++;

            if (ings->numOfIngs == arraySize) {
                arraySize *= 2;
                ings->arr = (Ingredient*)realloc(ings->arr, arraySize * sizeof(Ingredient));
            }
        }

        foods->numOfFoods++;

        if (foods->numOfFoods == arraySize) {
            arraySize *= 2;
            foods->food = (Food*)realloc(foods->food, arraySize * sizeof(Food));
        }

        ptr = ingEnd + 1;
    }
}

void showRecipes(Foods *foods){
    char msg[MAX_MSG];
    for (int i = 0; i < foods->numOfFoods; i++) {
        sprintf(msg,"%d- %s\n", i+1,foods->food[i].name);
        write(STDOUT_FILENO,msg,strlen(msg));
        for (int j = 0; j < foods->food[i].ings.numOfIngs; j++) {
            sprintf(msg,"        %s : %d\n", foods->food[i].ings.arr[j].name, foods->food[i].ings.arr[j].amount);
            write(STDOUT_FILENO,msg,strlen(msg));
        }
    }
}

void showIngredients(IngredientsArray *ingredients){
    char msg[MAX_MSG];
    sprintf(msg,"ingredient/amount\n");
    write(STDOUT_FILENO,msg,strlen(msg));
    for (int i = 0; i < ingredients->numOfIngs; i++) {
        if(ingredients->arr[i].amount>0){
            sprintf(msg,"%s %d\n", ingredients->arr[i].name,ingredients->arr[i].amount);
            write(STDOUT_FILENO,msg,strlen(msg));
        }
    }
}

void showFoods(Foods *foods){
    char msg[MAX_MSG];
    for (int i = 0; i < foods->numOfFoods; i++) {
        sprintf(msg,"%d- %s\n", i+1,foods->food[i].name);
        write(STDOUT_FILENO,msg,strlen(msg));
    }
}


void getFoods(Foods *foods){
    char* json_data=readFile();
    if (!strcmp(json_data,"failed"))   return;
    extractFoodsFromJson(json_data,foods);
    free(json_data);
}


void extractIngredients(const Foods* foods, IngredientsArray* ingredients) {
    if (ingredients->arr == NULL) {
        ingredients->arrSize = 10;
        ingredients->numOfIngs = 0;
        ingredients->arr = (Ingredient*)malloc(ingredients->arrSize * sizeof(Ingredient));
    }

    for (int i = 0; i < foods->numOfFoods; i++) {
        const Food* currentFood = &foods->food[i];
        for (int j = 0; j < currentFood->ings.numOfIngs; j++) {
            const Ingredient* currentIngredient = &currentFood->ings.arr[j];
            int isDuplicate = 0;
            for (int k = 0; k < ingredients->numOfIngs; k++) {
                if (strcmp(currentIngredient->name, ingredients->arr[k].name) == 0) {
                    isDuplicate = 1;
                    break;
                }
            }
            if (!isDuplicate) {
                if (ingredients->numOfIngs == ingredients->arrSize) {
                    ingredients->arrSize *= 2;
                    ingredients->arr = (Ingredient*)realloc(ingredients->arr, ingredients->arrSize * sizeof(Ingredient));
                }
                strcpy(ingredients->arr[ingredients->numOfIngs].name, currentIngredient->name);
                ingredients->arr[ingredients->numOfIngs].amount=0;
                ingredients->numOfIngs++;
            }
        }
    }
}
