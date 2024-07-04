import socket
import numpy as np
import time

def needleman_wunsch(seq1, seq2, match=1, mismatch=-1, gap=-1):
    start_time = time.time()
    len_seq1 = len(seq1)
    len_seq2 = len(seq2)
    score_matrix = np.zeros((len_seq1 + 1, len_seq2 + 1), dtype=int)
    for i in range(len_seq1 + 1):
        score_matrix[i][0] = gap * i
    for j in range(len_seq2 + 1):
        score_matrix[0][j] = gap * j
    for i in range(1, len_seq1 + 1):
        for j in range(1, len_seq2 + 1):
            match_score = score_matrix[i - 1][j - 1] + (match if seq1[i - 1] == seq2[j - 1] else mismatch)
            delete = score_matrix[i - 1][j] + gap
            insert = score_matrix[i][j - 1] + gap
            score_matrix[i][j] = max(match_score, delete, insert)
    align1, align2 = '', ''
    i, j = len_seq1, len_seq2
    while i > 0 or j > 0:
        current_score = score_matrix[i][j]
        if i > 0 and j > 0 and current_score == score_matrix[i - 1][j - 1] + (match if seq1[i - 1] == seq2[j - 1] else mismatch):
            align1 += seq1[i - 1]
            align2 += seq2[j - 1]
            i -= 1
            j -= 1
        elif i > 0 and current_score == score_matrix[i - 1][j] + gap:
            align1 += seq1[i - 1]
            align2 += '-'
            i -= 1
        else:
            align1 += '-'
            align2 += seq2[j - 1]
            j -= 1
    align1 = align1[::-1]
    align2 = align2[::-1]
    gaps = align1.count('-') + align2.count('-')
    score = score_matrix[len_seq1][len_seq2]
    time_taken = time.time() - start_time
    return align1, align2, gaps, score, time_taken

def smith_waterman(seq1, seq2, match=1, mismatch=-1, gap=-1):
    start_time = time.time()
    len_seq1 = len(seq1)
    len_seq2 = len(seq2)
    score_matrix = np.zeros((len_seq1 + 1, len_seq2 + 1), dtype=int)
    max_score = 0
    max_pos = None
    for i in range(1, len_seq1 + 1):
        for j in range(1, len_seq2 + 1):
            match_score = score_matrix[i - 1][j - 1] + (match if seq1[i - 1] == seq2[j - 1] else mismatch)
            delete = score_matrix[i - 1][j] + gap
            insert = score_matrix[i][j - 1] + gap
            score_matrix[i][j] = max(0, match_score, delete, insert)
            if score_matrix[i][j] >= max_score:
                max_score = score_matrix[i][j]
                max_pos = (i, j)
    align1, align2 = '', ''
    i, j = max_pos
    while score_matrix[i][j] != 0:
        current_score = score_matrix[i][j]
        if i > 0 and j > 0 and current_score == score_matrix[i - 1][j - 1] + (match if seq1[i - 1] == seq2[j - 1] else mismatch):
            align1 += seq1[i - 1]
            align2 += seq2[j - 1]
            i -= 1
            j -= 1
        elif i > 0 and current_score == score_matrix[i - 1][j] + gap:
            align1 += seq1[i - 1]
            align2 += '-'
            i -= 1
        else:
            align1 += '-'
            align2 += seq2[j - 1]
            j -= 1
    align1 = align1[::-1]
    align2 = align2[::-1]
    gaps = align1.count('-') + align2.count('-')
    score = max_score
    time_taken = time.time() - start_time
    return align1, align2, gaps, score, time_taken

def client_program():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(("127.0.0.1", 65432))
    message = client_socket.recv(4096).decode()
    seq1 = message.split(";")[0].split(":")[1]
    seq2 = message.split(";")[1].split(":")[1]
    nw_align1, nw_align2, nw_gaps, nw_score, nw_time = needleman_wunsch(seq1, seq2)
    sw_align1, sw_align2, sw_gaps, sw_score, sw_time = smith_waterman(seq1, seq2)
    result = f"Python;Needleman;Alignment1:{nw_align1};Alignment2:{nw_align2};AlignmentScore:{nw_score};Gap:{nw_gaps};ExecutionTime:{nw_time:.4f};Smith;Alignment1:{sw_align1};Alignment2:{sw_align2};AlignmentScore:{sw_score};Gap:{sw_gaps};ExecutionTime:{sw_time:.4f}"
    client_socket.send(result.encode())
    client_socket.close()

if __name__ == "__main__":
    client_program()