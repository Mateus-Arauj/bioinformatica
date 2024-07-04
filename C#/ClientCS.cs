using System;
using System.Net.Sockets;
using System.Text;

class NeedlemanWunsch
{
    public string Seq1 { get; }
    public string Seq2 { get; }
    public int Match { get; }
    public int Mismatch { get; }
    public int Gap { get; }

    public NeedlemanWunsch(string seq1, string seq2, int match, int mismatch, int gap)
    {
        Seq1 = seq1;
        Seq2 = seq2;
        Match = match;
        Mismatch = mismatch;
        Gap = gap;
    }

    public void Align(out string align1, out string align2, out int gaps, out int score, out double timeTaken)
    {
        var watch = System.Diagnostics.Stopwatch.StartNew();
        int lenSeq1 = Seq1.Length;
        int lenSeq2 = Seq2.Length;

        int[,] scoreMatrix = new int[lenSeq1 + 1, lenSeq2 + 1];

        for (int i = 0; i <= lenSeq1; ++i)
            scoreMatrix[i, 0] = Gap * i;
        for (int j = 0; j <= lenSeq2; ++j)
            scoreMatrix[0, j] = Gap * j;

        for (int i = 1; i <= lenSeq1; ++i)
        {
            for (int j = 1; j <= lenSeq2; ++j)
            {
                int matchScore = scoreMatrix[i - 1, j - 1] + (Seq1[i - 1] == Seq2[j - 1] ? Match : Mismatch);
                int deleteScore = scoreMatrix[i - 1, j] + Gap;
                int insertScore = scoreMatrix[i, j - 1] + Gap;
                scoreMatrix[i, j] = Math.Max(Math.Max(matchScore, deleteScore), insertScore);
            }
        }

        var align1Builder = new StringBuilder();
        var align2Builder = new StringBuilder();
        int idx1 = lenSeq1, idx2 = lenSeq2;

        while (idx1 > 0 || idx2 > 0)
        {
            if (idx1 > 0 && idx2 > 0 && scoreMatrix[idx1, idx2] == scoreMatrix[idx1 - 1, idx2 - 1] + (Seq1[idx1 - 1] == Seq2[idx2 - 1] ? Match : Mismatch))
            {
                align1Builder.Insert(0, Seq1[idx1 - 1]);
                align2Builder.Insert(0, Seq2[idx2 - 1]);
                --idx1;
                --idx2;
            }
            else if (idx1 > 0 && scoreMatrix[idx1, idx2] == scoreMatrix[idx1 - 1, idx2] + Gap)
            {
                align1Builder.Insert(0, Seq1[idx1 - 1]);
                align2Builder.Insert(0, '-');
                --idx1;
            }
            else
            {
                align1Builder.Insert(0, '-');
                align2Builder.Insert(0, Seq2[idx2 - 1]);
                --idx2;
            }
        }

        align1 = align1Builder.ToString();
        align2 = align2Builder.ToString();

        gaps = 0;
        for (int k = 0; k < align1.Length; ++k)
        {
            if (align1[k] == '-' || align2[k] == '-')
                ++gaps;
        }
        score = scoreMatrix[lenSeq1, lenSeq2];
        watch.Stop();
        timeTaken = watch.Elapsed.TotalSeconds;
    }
}

class SmithWaterman
{
    public string Seq1 { get; }
    public string Seq2 { get; }
    public int Match { get; }
    public int Mismatch { get; }
    public int Gap { get; }

    public SmithWaterman(string seq1, string seq2, int match, int mismatch, int gap)
    {
        Seq1 = seq1;
        Seq2 = seq2;
        Match = match;
        Mismatch = mismatch;
        Gap = gap;
    }

    public void Align(out string align1, out string align2, out int gaps, out int score, out double timeTaken)
    {
        var watch = System.Diagnostics.Stopwatch.StartNew();
        int lenSeq1 = Seq1.Length;
        int lenSeq2 = Seq2.Length;

        int[,] scoreMatrix = new int[lenSeq1 + 1, lenSeq2 + 1];
        int maxScore = 0, maxI = 0, maxJ = 0;

        for (int i = 1; i <= lenSeq1; ++i)
        {
            for (int j = 1; j <= lenSeq2; ++j)
            {
                int matchScore = scoreMatrix[i - 1, j - 1] + (Seq1[i - 1] == Seq2[j - 1] ? Match : Mismatch);
                int deleteScore = scoreMatrix[i - 1, j] + Gap;
                int insertScore = scoreMatrix[i, j - 1] + Gap;
                scoreMatrix[i, j] = Math.Max(0, Math.Max(Math.Max(matchScore, deleteScore), insertScore));
                if (scoreMatrix[i, j] > maxScore)
                {
                    maxScore = scoreMatrix[i, j];
                    maxI = i;
                    maxJ = j;
                }
            }
        }

        var align1Builder = new StringBuilder();
        var align2Builder = new StringBuilder();
        int idx1 = maxI, idx2 = maxJ;

        while (scoreMatrix[idx1, idx2] != 0)
        {
            if (idx1 > 0 && idx2 > 0 && scoreMatrix[idx1, idx2] == scoreMatrix[idx1 - 1, idx2 - 1] + (Seq1[idx1 - 1] == Seq2[idx2 - 1] ? Match : Mismatch))
            {
                align1Builder.Insert(0, Seq1[idx1 - 1]);
                align2Builder.Insert(0, Seq2[idx2 - 1]);
                --idx1;
                --idx2;
            }
            else if (idx1 > 0 && scoreMatrix[idx1, idx2] == scoreMatrix[idx1 - 1, idx2] + Gap)
            {
                align1Builder.Insert(0, Seq1[idx1 - 1]);
                align2Builder.Insert(0, '-');
                --idx1;
            }
            else
            {
                align1Builder.Insert(0, '-');
                align2Builder.Insert(0, Seq2[idx2 - 1]);
                --idx2;
            }
        }

        align1 = align1Builder.ToString();
        align2 = align2Builder.ToString();

        gaps = 0;
        for (int k = 0; k < align1.Length; ++k)
        {
            if (align1[k] == '-' || align2[k] == '-')
                ++gaps;
        }
        score = maxScore;
        watch.Stop();
        timeTaken = watch.Elapsed.TotalSeconds;
    }
}

class Program
{
    static void Main()
    {
        string server = "127.0.0.1";
        int port = 65432;

        using (var client = new TcpClient(server, port))
        using (var stream = client.GetStream())
        {
            byte[] buffer = new byte[4096];
            int bytesRead = stream.Read(buffer, 0, buffer.Length);
            string data = Encoding.ASCII.GetString(buffer, 0, bytesRead);

            string[] tokens = data.Split(';');
            string seq1 = tokens[0].Substring(6); // Skip "seq1:"
            string seq2 = tokens[1].Substring(6); // Skip "seq2:"

            NeedlemanWunsch nw = new NeedlemanWunsch(seq1, seq2, 1, -1, -1);
            nw.Align(out string nwAlign1, out string nwAlign2, out int nwGaps, out int nwScore, out double nwTime);

            SmithWaterman sw = new SmithWaterman(seq1, seq2, 1, -1, -1);
            sw.Align(out string swAlign1, out string swAlign2, out int swGaps, out int swScore, out double swTime);

            string result = $"C#;Needleman;Alignment1:{nwAlign1};Alignment2:{nwAlign2};AlignmentScore:{nwScore};Gap:{nwGaps};ExecutionTime:{nwTime:F4};Smith;Alignment1:{swAlign1};Alignment2:{swAlign2};AlignmentScore:{swScore};Gap:{swGaps};ExecutionTime:{swTime:F4}";
            byte[] resultBytes = Encoding.ASCII.GetBytes(result);
            stream.Write(resultBytes, 0, resultBytes.Length);
        }
    }
}