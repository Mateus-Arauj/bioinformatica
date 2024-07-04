#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

using namespace std;
using namespace std::chrono;

class NeedlemanWunsch {
public:
    NeedlemanWunsch(const string& seq1, const string& seq2, int match = 1, int mismatch = -1, int gap = -1)
        : seq1(seq1), seq2(seq2), match(match), mismatch(mismatch), gap(gap) {}

    tuple<string, string, int, int, double> align() {
        auto start = high_resolution_clock::now();
        int len_seq1 = seq1.length();
        int len_seq2 = seq2.length();

        vector<vector<int>> score_matrix(len_seq1 + 1, vector<int>(len_seq2 + 1, 0));

        for (int i = 0; i <= len_seq1; ++i) {
            score_matrix[i][0] = gap * i;
        }
        for (int j = 0; j <= len_seq2; ++j) {
            score_matrix[0][j] = gap * j;
        }

        for (int i = 1; i <= len_seq1; ++i) {
            for (int j = 1; j <= len_seq2; ++j) {
                int match_score = score_matrix[i - 1][j - 1] + (seq1[i - 1] == seq2[j - 1] ? match : mismatch);
                int delete_score = score_matrix[i - 1][j] + gap;
                int insert_score = score_matrix[i][j - 1] + gap;
                score_matrix[i][j] = max({match_score, delete_score, insert_score});
            }
        }

        string align1 = "", align2 = "";
        int i = len_seq1, j = len_seq2;
        while (i > 0 || j > 0) {
            int current_score = score_matrix[i][j];
            if (i > 0 && j > 0 && current_score == score_matrix[i - 1][j - 1] + (seq1[i - 1] == seq2[j - 1] ? match : mismatch)) {
                align1 += seq1[i - 1];
                align2 += seq2[j - 1];
                --i;
                --j;
            } else if (i > 0 && current_score == score_matrix[i - 1][j] + gap) {
                align1 += seq1[i - 1];
                align2 += '-';
                --i;
            } else {
                align1 += '-';
                align2 += seq2[j - 1];
                --j;
            }
        }

        reverse(align1.begin(), align1.end());
        reverse(align2.begin(), align2.end());
        int gaps = count(align1.begin(), align1.end(), '-') + count(align2.begin(), align2.end(), '-');
        int score = score_matrix[len_seq1][len_seq2];
        auto end = high_resolution_clock::now();
        double time_taken = duration_cast<duration<double>>(end - start).count();

        return make_tuple(align1, align2, gaps, score, time_taken);
    }

private:
    string seq1, seq2;
    int match, mismatch, gap;
};

class SmithWaterman {
public:
    SmithWaterman(const string& seq1, const string& seq2, int match = 1, int mismatch = -1, int gap = -1)
        : seq1(seq1), seq2(seq2), match(match), mismatch(mismatch), gap(gap) {}

    tuple<string, string, int, int, double> align() {
        auto start = high_resolution_clock::now();
        int len_seq1 = seq1.length();
        int len_seq2 = seq2.length();

        vector<vector<int>> score_matrix(len_seq1 + 1, vector<int>(len_seq2 + 1, 0));

        int max_score = 0;
        pair<int, int> max_pos = {0, 0};
        for (int i = 1; i <= len_seq1; ++i) {
            for (int j = 1; j <= len_seq2; ++j) {
                int match_score = score_matrix[i - 1][j - 1] + (seq1[i - 1] == seq2[j - 1] ? match : mismatch);
                int delete_score = score_matrix[i - 1][j] + gap;
                int insert_score = score_matrix[i][j - 1] + gap;
                score_matrix[i][j] = max({0, match_score, delete_score, insert_score});
                if (score_matrix[i][j] >= max_score) {
                    max_score = score_matrix[i][j];
                    max_pos = {i, j};
                }
            }
        }

        string align1 = "", align2 = "";
        int i = max_pos.first, j = max_pos.second;
        while (score_matrix[i][j] != 0) {
            int current_score = score_matrix[i][j];
            if (i > 0 && j > 0 && current_score == score_matrix[i - 1][j - 1] + (seq1[i - 1] == seq2[j - 1] ? match : mismatch)) {
                align1 += seq1[i - 1];
                align2 += seq2[j - 1];
                --i;
                --j;
            } else if (i > 0 && current_score == score_matrix[i - 1][j] + gap) {
                align1 += seq1[i - 1];
                align2 += '-';
                --i;
            } else {
                align1 += '-';
                align2 += seq2[j - 1];
                --j;
            }
        }

        reverse(align1.begin(), align1.end());
        reverse(align2.begin(), align2.end());
        int gaps = count(align1.begin(), align1.end(), '-') + count(align2.begin(), align2.end(), '-');
        int score = max_score;
        auto end = high_resolution_clock::now();
        double time_taken = duration_cast<duration<double>>(end - start).count();

        return make_tuple(align1, align2, gaps, score, time_taken);
    }

private:
    string seq1, seq2;
    int match, mismatch, gap;
};

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
    string data(buffer);
    string seq1 = data.substr(data.find("seq1:") + 5, data.find(";seq2:") - data.find("seq1:") - 5);
    string seq2 = data.substr(data.find("seq2:") + 5);

    NeedlemanWunsch nw(seq1, seq2);
    auto [nw_align1, nw_align2, nw_gaps, nw_score, nw_time] = nw.align();

    SmithWaterman sw(seq1, seq2);
    auto [sw_align1, sw_align2, sw_gaps, sw_score, sw_time] = sw.align();

    snprintf(buffer, BUFFER_SIZE, "C++;Needleman;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f;Smith;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f",
             nw_align1.c_str(), nw_align2.c_str(), nw_score, nw_gaps, nw_time, sw_align1.c_str(), sw_align2.c_str(), sw_score, sw_gaps, sw_time);

    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
}

int main() {
    client_program();
    return 0;
}