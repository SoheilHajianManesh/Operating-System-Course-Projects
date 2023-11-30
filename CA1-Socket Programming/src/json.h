#ifndef JSON_H_INCLUDE
#define JSON_H_INCLUDE

#include "defs.h"

char* readFile();
void extractFoodsFromJson(const char* json_data, Foods* foods);
void showRecipes(Foods *foods);
void showIngredients(IngredientsArray* ingredients);
void showFoods(Foods *foods);
void getFoods(Foods *foods);
void extractIngredients(const Foods* foods, IngredientsArray* ingredients);

#endif