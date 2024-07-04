#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define BUFFER_SIZE 4096

typedef struct {
    char* seq1;
    char* seq2;
    int match;
    int mismatch;
    int gap;
} NeedlemanWunsch;

NeedlemanWunsch* createNeedlemanWunsch(const char* seq1, const char* seq2, int match, int mismatch, int gap) {
    NeedlemanWunsch* nw = (NeedlemanWunsch*)malloc(sizeof(NeedlemanWunsch));
    nw->seq1 = strdup(seq1);
    nw->seq2 = strdup(seq2);
    nw->match = match;
    nw->mismatch = mismatch;
    nw->gap = gap;
    return nw;
}

void freeNeedlemanWunsch(NeedlemanWunsch* nw) {
    free(nw->seq1);
    free(nw->seq2);
    free(nw);
}

void alignNeedlemanWunsch(NeedlemanWunsch* nw, char** align1, char** align2, int* gaps, int* score, double* time_taken) {
    clock_t start = clock();
    int len_seq1 = strlen(nw->seq1);
    int len_seq2 = strlen(nw->seq2);

    int** score_matrix = (int**)malloc((len_seq1 + 1) * sizeof(int*));
    for (int i = 0; i <= len_seq1; ++i) {
        score_matrix[i] = (int*)malloc((len_seq2 + 1) * sizeof(int));
        if (i == 0) {
            for (int j = 0; j <= len_seq2; ++j) {
                score_matrix[i][j] = nw->gap * j;
            }
        } else {
            score_matrix[i][0] = nw->gap * i;
        }
    }

    for (int i = 1; i <= len_seq1; ++i) {
        for (int j = 1; j <= len_seq2; ++j) {
            int match_score = score_matrix[i - 1][j - 1] + (nw->seq1[i - 1] == nw->seq2[j - 1] ? nw->match : nw->mismatch);
            int delete_score = score_matrix[i - 1][j] + nw->gap;
            int insert_score = score_matrix[i][j - 1] + nw->gap;
            score_matrix[i][j] = MAX(MAX(match_score, delete_score), insert_score);
        }
    }

    *align1 = (char*)malloc((len_seq1 + len_seq2 + 1) * sizeof(char));
    *align2 = (char*)malloc((len_seq1 + len_seq2 + 1) * sizeof(char));
    int idx1 = 0, idx2 = 0;
    int i = len_seq1, j = len_seq2;
    while (i > 0 || j > 0) {
        int current_score = score_matrix[i][j];
        if (i > 0 && j > 0 && current_score == score_matrix[i - 1][j - 1] + (nw->seq1[i - 1] == nw->seq2[j - 1] ? nw->match : nw->mismatch)) {
            (*align1)[idx1++] = nw->seq1[i - 1];
            (*align2)[idx2++] = nw->seq2[j - 1];
            --i;
            --j;
        } else if (i > 0 && current_score == score_matrix[i - 1][j] + nw->gap) {
            (*align1)[idx1++] = nw->seq1[i - 1];
            (*align2)[idx2++] = '-';
            --i;
        } else {
            (*align1)[idx1++] = '-';
            (*align2)[idx2++] = nw->seq2[j - 1];
            --j;
        }
    }
    (*align1)[idx1] = '\0';
    (*align2)[idx2] = '\0';

    for (int k = 0; k < idx1 / 2; ++k) {
        char temp = (*align1)[k];
        (*align1)[k] = (*align1)[idx1 - k - 1];
        (*align1)[idx1 - k - 1] = temp;
    }
    for (int k = 0; k < idx2 / 2; ++k) {
        char temp = (*align2)[k];
        (*align2)[k] = (*align2)[idx2 - k - 1];
        (*align2)[idx2 - k - 1] = temp;
    }

    *gaps = 0;
    for (int k = 0; k < idx1; ++k) {
        if ((*align1)[k] == '-' || (*align2)[k] == '-') {
            (*gaps)++;
        }
    }
    *score = score_matrix[len_seq1][len_seq2];
    *time_taken = ((double)(clock() - start)) / CLOCKS_PER_SEC;

    for (int i = 0; i <= len_seq1; ++i) {
        free(score_matrix[i]);
    }
    free(score_matrix);
}

typedef struct {
    char* seq1;
    char* seq2;
    int match;
    int mismatch;
    int gap;
} SmithWaterman;

SmithWaterman* createSmithWaterman(const char* seq1, const char* seq2, int match, int mismatch, int gap) {
    SmithWaterman* sw = (SmithWaterman*)malloc(sizeof(SmithWaterman));
    sw->seq1 = strdup(seq1);
    sw->seq2 = strdup(seq2);
    sw->match = match;
    sw->mismatch = mismatch;
    sw->gap = gap;
    return sw;
}

void freeSmithWaterman(SmithWaterman* sw) {
    free(sw->seq1);
    free(sw->seq2);
    free(sw);
}

void alignSmithWaterman(SmithWaterman* sw, char** align1, char** align2, int* gaps, int* score, double* time_taken) {
    clock_t start = clock();
    int len_seq1 = strlen(sw->seq1);
    int len_seq2 = strlen(sw->seq2);

    int** score_matrix = (int**)malloc((len_seq1 + 1) * sizeof(int*));
    for (int i = 0; i <= len_seq1; ++i) {
        score_matrix[i] = (int*)malloc((len_seq2 + 1) * sizeof(int));
        for (int j = 0; j <= len_seq2; ++j) {
            score_matrix[i][j] = 0;
        }
    }

    int max_score = 0;
    int max_i = 0, max_j = 0;
    for (int i = 1; i <= len_seq1; ++i) {
        for (int j = 1; j <= len_seq2; ++j) {
            int match_score = score_matrix[i - 1][j - 1] + (sw->seq1[i - 1] == sw->seq2[j - 1] ? sw->match : sw->mismatch);
            int delete_score = score_matrix[i - 1][j] + sw->gap;
            int insert_score = score_matrix[i][j - 1] + sw->gap;
            score_matrix[i][j] = MAX(MAX(match_score, delete_score), insert_score);
            score_matrix[i][j] = MAX(score_matrix[i][j], 0);
            if (score_matrix[i][j] >= max_score) {
                max_score = score_matrix[i][j];
                max_i = i;
                max_j = j;
            }
        }
    }

    *align1 = (char*)malloc((len_seq1 + len_seq2 + 1) * sizeof(char));
    *align2 = (char*)malloc((len_seq1 + len_seq2 + 1) * sizeof(char));
    int idx1 = 0, idx2 = 0;
    int i = max_i, j = max_j;
    while (score_matrix[i][j] != 0) {
        int current_score = score_matrix[i][j];
        if (i > 0 && j > 0 && current_score == score_matrix[i - 1][j - 1] + (sw->seq1[i - 1] == sw->seq2[j - 1] ? sw->match : sw->mismatch)) {
            (*align1)[idx1++] = sw->seq1[i - 1];
            (*align2)[idx2++] = sw->seq2[j - 1];
            --i;
            --j;
        } else if (i > 0 && current_score == score_matrix[i - 1][j] + sw->gap) {
            (*align1)[idx1++] = sw->seq1[i - 1];
            (*align2)[idx2++] = '-';
            --i;
        } else {
            (*align1)[idx1++] = '-';
            (*align2)[idx2++] = sw->seq2[j - 1];
            --j;
        }
    }
    (*align1)[idx1] = '\0';
    (*align2)[idx2] = '\0';

    for (int k = 0; k < idx1 / 2; ++k) {
        char temp = (*align1)[k];
        (*align1)[k] = (*align1)[idx1 - k - 1];
        (*align1)[idx1 - k - 1] = temp;
    }
    for (int k = 0; k < idx2 / 2; ++k) {
        char temp = (*align2)[k];
        (*align2)[k] = (*align2)[idx2 - k - 1];
        (*align2)[idx2 - k - 1] = temp;
    }

    *gaps = 0;
    for (int k = 0; k < idx1; ++k) {
        if ((*align1)[k] == '-' || (*align2)[k] == '-') {
            (*gaps)++;
        }
    }
    *score = max_score;
    *time_taken = ((double)(clock() - start)) / CLOCKS_PER_SEC;

    for (int i = 0; i <= len_seq1; ++i) {
        free(score_matrix[i]);
    }
    free(score_matrix);
}

void client_program() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(65432);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0';
    char* token = strtok(buffer, ";");
    char* seq1 = strdup(token + 6); // Skip "seq1:"
    token = strtok(NULL, ";");
    char* seq2 = strdup(token + 6); // Skip "seq2:"

    char* nw_align1;
    char* nw_align2;
    int nw_gaps;
    int nw_score;
    double nw_time;
    NeedlemanWunsch* nw = createNeedlemanWunsch(seq1, seq2, 1, -1, -1);
    alignNeedlemanWunsch(nw, &nw_align1, &nw_align2, &nw_gaps, &nw_score, &nw_time);
    freeNeedlemanWunsch(nw);

    char* sw_align1;
    char* sw_align2;
    int sw_gaps;
    int sw_score;
    double sw_time;
    SmithWaterman* sw = createSmithWaterman(seq1, seq2, 1, -1, -1);
    alignSmithWaterman(sw, &sw_align1, &sw_align2, &sw_gaps, &sw_score, &sw_time);
    freeSmithWaterman(sw);

    char result[BUFFER_SIZE];
    snprintf(result, BUFFER_SIZE, "C;Needleman;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f;Smith;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f",
             nw_align1, nw_align2, nw_score, nw_gaps, nw_time, sw_align1, sw_align2, sw_score, sw_gaps, sw_time);

    send(client_socket, result, strlen(result), 0);
    close(client_socket);

    free(nw_align1);
    free(nw_align2);
    free(sw_align1);
    free(sw_align2);
    free(seq1);
    free(seq2);
}

int main() {
    client_program();
    return 0;
}