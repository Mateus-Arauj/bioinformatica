import java.io.*;
import java.net.Socket;

public class Client {

    public static void main(String[] args) {
        String serverAddress = "127.0.0.1";
        int port = 65432;

        try (Socket socket = new Socket(serverAddress, port);
             InputStream input = socket.getInputStream();
             OutputStream output = socket.getOutputStream();
             BufferedReader reader = new BufferedReader(new InputStreamReader(input));
             PrintWriter writer = new PrintWriter(output, true)) {

            char[] buffer = new char[4096];
            int bytesRead = reader.read(buffer);
            String data = new String(buffer, 0, bytesRead);

            String[] tokens = data.split(";");
            String seq1 = tokens[0].substring(6); 
            String seq2 = tokens[1].substring(6); 

            NeedlemanWunsch nw = new NeedlemanWunsch(seq1, seq2, 1, -1, -1);
            AlignmentResult nwResult = nw.align();

            SmithWaterman sw = new SmithWaterman(seq1, seq2, 1, -1, -1);
            AlignmentResult swResult = sw.align();

            String result = String.format("C;Needleman;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f;Smith;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f",
                    nwResult.alignment1, nwResult.alignment2, nwResult.score, nwResult.gaps, nwResult.timeTaken,
                    swResult.alignment1, swResult.alignment2, swResult.score, swResult.gaps, swResult.timeTaken);

            writer.println(result);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

class NeedlemanWunsch {
    private final String seq1;
    private final String seq2;
    private final int match;
    private final int mismatch;
    private final int gap;

    public NeedlemanWunsch(String seq1, String seq2, int match, int mismatch, int gap) {
        this.seq1 = seq1;
        this.seq2 = seq2;
        this.match = match;
        this.mismatch = mismatch;
        this.gap = gap;
    }

    public AlignmentResult align() {
        long startTime = System.currentTimeMillis();
        int lenSeq1 = seq1.length();
        int lenSeq2 = seq2.length();
        int[][] scoreMatrix = new int[lenSeq1 + 1][lenSeq2 + 1];

        for (int i = 0; i <= lenSeq1; ++i) {
            scoreMatrix[i][0] = gap * i;
        }
        for (int j = 0; j <= lenSeq2; ++j) {
            scoreMatrix[0][j] = gap * j;
        }

        for (int i = 1; i <= lenSeq1; ++i) {
            for (int j = 1; j <= lenSeq2; ++j) {
                int matchScore = scoreMatrix[i - 1][j - 1] + (seq1.charAt(i - 1) == seq2.charAt(j - 1) ? match : mismatch);
                int deleteScore = scoreMatrix[i - 1][j] + gap;
                int insertScore = scoreMatrix[i][j - 1] + gap;
                scoreMatrix[i][j] = Math.max(Math.max(matchScore, deleteScore), insertScore);
            }
        }

        StringBuilder align1 = new StringBuilder();
        StringBuilder align2 = new StringBuilder();
        int i = lenSeq1, j = lenSeq2;
        while (i > 0 || j > 0) {
            if (i > 0 && j > 0 && scoreMatrix[i][j] == scoreMatrix[i - 1][j - 1] + (seq1.charAt(i - 1) == seq2.charAt(j - 1) ? match : mismatch)) {
                align1.append(seq1.charAt(i - 1));
                align2.append(seq2.charAt(j - 1));
                --i;
                --j;
            } else if (i > 0 && scoreMatrix[i][j] == scoreMatrix[i - 1][j] + gap) {
                align1.append(seq1.charAt(i - 1));
                align2.append('-');
                --i;
            } else {
                align1.append('-');
                align2.append(seq2.charAt(j - 1));
                --j;
            }
        }

        String alignedSeq1 = align1.reverse().toString();
        String alignedSeq2 = align2.reverse().toString();

        int gaps = 0;
        for (int k = 0; k < alignedSeq1.length(); ++k) {
            if (alignedSeq1.charAt(k) == '-' || alignedSeq2.charAt(k) == '-') {
                ++gaps;
            }
        }

        int score = scoreMatrix[lenSeq1][lenSeq2];
        double timeTaken = (System.currentTimeMillis() - startTime) / 1000.0;

        return new AlignmentResult(alignedSeq1, alignedSeq2, gaps, score, timeTaken);
    }
}

class SmithWaterman {
    private final String seq1;
    private final String seq2;
    private final int match;
    private final int mismatch;
    private final int gap;

    public SmithWaterman(String seq1, String seq2, int match, int mismatch, int gap) {
        this.seq1 = seq1;
        this.seq2 = seq2;
        this.match = match;
        this.mismatch = mismatch;
        this.gap = gap;
    }

    public AlignmentResult align() {
        long startTime = System.currentTimeMillis();
        int lenSeq1 = seq1.length();
        int lenSeq2 = seq2.length();
        int[][] scoreMatrix = new int[lenSeq1 + 1][lenSeq2 + 1];
        int maxScore = 0, maxI = 0, maxJ = 0;

        for (int i = 1; i <= lenSeq1; ++i) {
            for (int j = 1; j <= lenSeq2; ++j) {
                int matchScore = scoreMatrix[i - 1][j - 1] + (seq1.charAt(i - 1) == seq2.charAt(j - 1) ? match : mismatch);
                int deleteScore = scoreMatrix[i - 1][j] + gap;
                int insertScore = scoreMatrix[i][j - 1] + gap;
                scoreMatrix[i][j] = Math.max(0, Math.max(Math.max(matchScore, deleteScore), insertScore));
                if (scoreMatrix[i][j] >= maxScore) {
                    maxScore = scoreMatrix[i][j];
                    maxI = i;
                    maxJ = j;
                }
            }
        }

        StringBuilder align1 = new StringBuilder();
        StringBuilder align2 = new StringBuilder();
        int i = maxI, j = maxJ;
        while (scoreMatrix[i][j] != 0) {
            if (i > 0 && j > 0 && scoreMatrix[i][j] == scoreMatrix[i - 1][j - 1] + (seq1.charAt(i - 1) == seq2.charAt(j - 1) ? match : mismatch)) {
                align1.append(seq1.charAt(i - 1));
                align2.append(seq2.charAt(j - 1));
                --i;
                --j;
            } else if (i > 0 && scoreMatrix[i][j] == scoreMatrix[i - 1][j] + gap) {
                align1.append(seq1.charAt(i - 1));
                align2.append('-');
                --i;
            } else {
                align1.append('-');
                align2.append(seq2.charAt(j - 1));
                --j;
            }
        }

        String alignedSeq1 = align1.reverse().toString();
        String alignedSeq2 = align2.reverse().toString();

        int gaps = 0;
        for (int k = 0; k < alignedSeq1.length(); ++k) {
            if (alignedSeq1.charAt(k) == '-' || alignedSeq2.charAt(k) == '-') {
                ++gaps;
            }
        }

        double timeTaken = (System.currentTimeMillis() - startTime) / 1000.0;

        return new AlignmentResult(alignedSeq1, alignedSeq2, gaps, maxScore, timeTaken);
    }
}

class AlignmentResult {
    public final String alignment1;
    public final String alignment2;
    public final int gaps;
    public final int score;
    public final double timeTaken;

    public AlignmentResult(String alignment1, String alignment2, int gaps, int score, double timeTaken) {
        this.alignment1 = alignment1;
        this.alignment2 = alignment2;
        this.gaps = gaps;
        this.score = score;
        this.timeTaken = timeTaken;
    }
}