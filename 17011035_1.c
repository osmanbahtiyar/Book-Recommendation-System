#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_LENGTH 50

typedef struct user
{
    int *books;
    float *similarity;
    int *similarity_id;
} User;

User **train_data = NULL;
User **test_data = NULL;
char **book_names = NULL;
int book_count = 0;
int train_count = 0;
int test_count = 0;
int k;

void readFromCSV();
void expandBookNames();
User *createNewUser();
void expandTrainData();
void expandTestData();

int main()
{

    readFromCSV();

    // printf("Train count -> %d\nTest count -> %d", train_count, test_count);

    // printf("Train data\n");
    // int i;
    // int j;
    // for (i = 0; i < train_count; i++)
    // {
    //     printf("User %d : ", i);
    //     for (j = 0; j < book_count; j++)
    //     {
    //         printf(" %d ", train_data[i]->books[j]);
    //     }
    //     printf("\n");
    // }

    // for (i = 0; i < test_count; i++)
    // {
    //     printf("User %d : ", i);
    //     for (j = 0; j < book_count; j++)
    //     {
    //         printf(" %d ", test_data[i]->books[j]);
    //     }
    //     printf("\n");
    // }

    return 0;
}

void readFromCSV()
{
    FILE *fp = fopen("RecomendationDataSet.csv", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File error");
        exit(1);
    }

    char line[1000];

    fgets(line, sizeof(line), fp);
    char *token;
    token = strtok(line, "\n");
    token = strtok(line, ",");
    token = strtok(NULL, ",");
    while (token != NULL)
    {
        book_count++;
        expandBookNames();
        strcpy(book_names[book_count - 1], token);
        token = strtok(NULL, ",");
    }

    int i_train = 0;
    int i_test = 0;
    int j;
    while (fgets(line, sizeof(line), fp))
    {
        token = strtok(line, "\n");
        token = strtok(line, ",");
        char user_type = token[0];
        token = strtok(NULL, ","); //first rate
        if (user_type == 'U' || user_type == 'u')
        {
            train_count++;
            expandTrainData();

            train_data[train_count - 1] = createNewUser();

            j = 0;
            while (token != NULL)
            {
                train_data[i_train]->books[j] = atoi(token);
                j++;
                token = strtok(NULL, ",");
            }
            i_train++;
        }
        else
        {
            test_count++;
            expandTestData();
            test_data[test_count - 1] = createNewUser();
            j = 0;
            while (token != NULL)
            {
                test_data[i_test]->books[j] = atoi(token);
                j++;
                token = strtok(NULL, ",");
            }
            i_test++;
        }
    }
}

void expandTestData()
{
    User **tmp_data = (User **)realloc(test_data, test_count * sizeof(User *));
    if (tmp_data == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    else
    {
        test_data = tmp_data;
    }
}

void expandTrainData()
{
    User **tmp_data = (User **)realloc(train_data, train_count * sizeof(User *));
    if (tmp_data == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    else
    {
        train_data = tmp_data;
    }
}

void expandBookNames()
{
    char **new_book_names = (char **)realloc(book_names, book_count * sizeof(char *));
    if (new_book_names == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    else
    {
        book_names = new_book_names;
    }

    book_names[book_count - 1] = (char *)malloc(MAX_LENGTH * sizeof(char));
    if (book_names[book_count - 1] == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
}

User *createNewUser()
{
    User *newUser = (User *)malloc(sizeof(User));
    if (newUser == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }

    newUser->books = (int *)malloc(book_count * sizeof(int));
    newUser->similarity = (float *)malloc(k * sizeof(float));
    newUser->similarity_id = (int *)malloc(k * sizeof(int));
    if (newUser->books == NULL || newUser->similarity == NULL || newUser->similarity_id == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    int i;
    for (i = 0; i < k; i++)
    {
        newUser->similarity[i] = -1;
        newUser->similarity_id[i] = -1;
    }
    return newUser;
}
