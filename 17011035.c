#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
    int user_id;
    float similarity;
} Sim_node;

void readFromCSV(int **train_data, int train_size, int **test_data, int test_size, char **book_names, int book_count);
float calculateSimilarity(int *user1, int *user2, int book_count);
void shiftRight(Sim_node *arr, int n, int index);
void insertToSimilarityArray(Sim_node *similarity_arr, int k, float new_sim, int new_id);
void findKSimilarUsers(int *new_user, Sim_node *nu_similarity, int **train_data, int train_size, int book_count, int k);
float calculateAverageRating(int *user, int book_count);
float makePrediction(int *nu, int book_no, Sim_node *nu_similarity, int **train_data, int book_count, int k);

int main()
{
    int train_size;
    int test_size;
    int book_count;
    int k;

    int **train_data;
    int **test_data;
    char **book_names;
    Sim_node **similarity;

    printf("Train data kullanici sayisini giriniz: ");
    scanf("%d", &train_size);
    printf("Test data kullanici sayisini giriniz: ");
    scanf("%d", &test_size);
    printf("Kitap sayisini giriniz: ");
    scanf("%d", &book_count);
    printf("En benzer kac kisiyi istiyorsunuz: ");
    scanf("%d", &k);

    train_data = (int **)calloc(train_size, sizeof(int *));
    test_data = (int **)calloc(test_size, sizeof(int *));
    book_names = (char **)calloc(book_count, sizeof(char *));
    similarity = (Sim_node **)calloc(test_size, sizeof(Sim_node *));
    if (train_data == NULL || test_data == NULL || book_names == NULL || similarity == NULL)
    {
        fprintf(stderr, "Allocation Error\n");
        exit(1);
    }

    int i;
    for (i = 0; i < train_size; i++)
    {
        train_data[i] = (int *)calloc(book_count, sizeof(int));
        if (train_data[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < test_size; i++)
    {
        test_data[i] = (int *)calloc(test_size, sizeof(int));
        if (test_data[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < book_count; i++)
    {
        book_names[i] = (char *)calloc(50, sizeof(char));
        if (book_names[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < test_size; i++)
    {
        similarity[i] = (Sim_node *)calloc(k, sizeof(Sim_node));
        if (similarity[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }

    readFromCSV(train_data, train_size, test_data, test_size, book_names, book_count);
    printf("\n\n");
    for (i = 0; i < test_size; i++)
    {
        findKSimilarUsers(test_data[i], similarity[i], train_data, train_size, book_count, k);
    }
    for (i = 0; i < test_size; i++)
    {
        int j;
        printf("NU%d icin benzerlik oranlari:\n", i + 1);
        for (j = 0; j < k; j++)
        {
            printf("%d. en yakin id: %d, benzerlik: %f\n", j + 1, similarity[i][j].user_id + 1, similarity[i][j].similarity);
        }
    }

    printf("\n\n");
    for (i = 0; i < test_size; i++)
    {
        int j;
        float max_rate = -1;
        int max_book;
        printf("NU%d icin muhtemel kitap puanlari:\n", i + 1);
        for (j = 0; j < book_count; j++)
        {
            if (test_data[i][j] == 0)
            {
                float rate = makePrediction(test_data[i], j, similarity[i], train_data, book_count, k);
                printf("%s kitabi icin beklenen puan : %f\n", book_names[j], rate);
                if (rate > max_rate)
                {
                    max_rate = rate;
                    max_book = j;
                }
            }
        }
        printf("Biz %s kitabini oneriyoruz!\n", book_names[max_book]);
        printf("\n");
    }

    return 0;
}

float calculateAverageRating(int *user, int book_count)
{
    int i;
    int user_count = 0;
    float avg = 0;
    for (i = 0; i < book_count; i++)
    {
        if (user[i] != 0)
        {
            avg += user[i];
            user_count++;
        }
    }
    return avg / user_count;
}

float makePrediction(int *nu, int book_no, Sim_node *nu_similarity, int **train_data, int book_count, int k)
{
    int i;
    float rate = 0;
    float avg_nu = calculateAverageRating(nu, book_count);
    float term1 = 0;
    float term2 = 0;
    for (i = 0; i < k; i++)
    {
        float avg_u = calculateAverageRating(train_data[nu_similarity[i].user_id], book_count);
        float sim = calculateSimilarity(nu, train_data[nu_similarity[i].user_id], book_count);
        term1 += sim * (train_data[nu_similarity[i].user_id][book_no] - avg_u);
        term2 += sim;
    }
    return avg_nu + (term1 / term2);
}

void findKSimilarUsers(int *new_user, Sim_node *nu_similarity, int **train_data, int train_size, int book_count, int k)
{
    int i;
    for (i = 0; i < k; i++)
    {
        nu_similarity[i].similarity = -1;
    }

    for (i = 0; i < train_size; i++)
    {
        float sim = calculateSimilarity(new_user, train_data[i], book_count);
        insertToSimilarityArray(nu_similarity, k, sim, i);
    }
}

void insertToSimilarityArray(Sim_node *similarity_arr, int k, float new_sim, int new_id)
{
    int i;
    for (i = 0; i < k; i++)
    {
        if (new_sim > similarity_arr[i].similarity)
        {
            shiftRight(similarity_arr, k, i);
            similarity_arr[i].similarity = new_sim;
            similarity_arr[i].user_id = new_id;
            return;
        }
    }
}

void shiftRight(Sim_node *arr, int n, int index)
{
    int i;
    for (i = n - 1; i > index; i--)
    {
        arr[i].similarity = arr[i - 1].similarity;
        arr[i].user_id = arr[i - 1].user_id;
    }
}

float calculateSimilarity(int *user1, int *user2, int book_count)
{
    int i;
    float sim = 0;
    float avg_user1 = 0;
    int user1_count = 0;
    float avg_user2 = 0;
    int user2_count = 0;
    for (i = 0; i < book_count; i++)
    {
        if (user1[i] != 0)
        {
            avg_user1 += user1[i];
            user1_count++;
        }
        if (user2[i] != 0)
        {
            avg_user2 += user2[i];
            user2_count++;
        }
    }
    avg_user1 /= user1_count;
    avg_user2 /= user2_count;

    float cov = 0;
    float std_user1 = 0;
    float std_user2 = 0;
    for (i = 0; i < book_count; i++)
    {
        if (user1[i] != 0 && user2[i] != 0)
        {
            cov += (user1[i] - avg_user1) * (user2[i] - avg_user2);
            std_user1 += (user1[i] - avg_user1) * (user1[i] - avg_user1);
            std_user2 += (user2[i] - avg_user2) * (user2[i] - avg_user2);
        }
    }
    std_user1 = sqrt(std_user1);
    std_user2 = sqrt(std_user2);
    sim = cov / (std_user1 * std_user2);
    return sim;
}

void readFromCSV(int **train_data, int train_size, int **test_data, int test_size, char **book_names, int book_count)
{
    FILE *fp = fopen("RecomendationDataSet.csv", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File error");
        exit(1);
    }

    char line[500];

    fgets(line, sizeof(line), fp);
    char *token;
    int i = 0;
    token = strtok(line, "\n");
    token = strtok(line, ",");
    token = strtok(NULL, ",");
    while (token != NULL)
    {
        strcpy(book_names[i++], token);
        token = strtok(NULL, ",");
    }

    for (i = 0; i < train_size; i++)
    {
        fgets(line, sizeof(line), fp);
        token = strtok(line, "\n");
        token = strtok(line, ",");
        token = strtok(NULL, ",");
        int j = 0;
        while (token != NULL)
        {
            train_data[i][j++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }

    fgets(line, sizeof(line), fp);

    for (i = 0; i < test_size; i++)
    {
        fgets(line, sizeof(line), fp);
        token = strtok(line, "\n");
        token = strtok(line, ",");
        token = strtok(NULL, ",");
        int j = 0;
        while (token != NULL)
        {
            test_data[i][j++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }
}
