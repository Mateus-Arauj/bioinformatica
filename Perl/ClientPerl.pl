use strict;
use warnings;
use IO::Socket::INET;
use Time::HiRes qw(gettimeofday tv_interval);


sub needleman_wunsch {
    my ($seq1, $seq2, $match, $mismatch, $gap) = @_;
    my $start_time = [gettimeofday];
    my $len_seq1 = length($seq1);
    my $len_seq2 = length($seq2);
    my @score_matrix;

    for my $i (0..$len_seq1) {
        $score_matrix[$i][0] = $gap * $i;
    }
    for my $j (0..$len_seq2) {
        $score_matrix[0][$j] = $gap * $j;
    }

    for my $i (1..$len_seq1) {
        for my $j (1..$len_seq2) {
            my $match_score = $score_matrix[$i-1][$j-1] + (($seq1 =~ /./g)[$i-1] eq ($seq2 =~ /./g)[$j-1] ? $match : $mismatch);
            my $delete_score = $score_matrix[$i-1][$j] + $gap;
            my $insert_score = $score_matrix[$i][$j-1] + $gap;
            $score_matrix[$i][$j] = max($match_score, $delete_score, $insert_score);
        }
    }

    my ($align1, $align2) = ('', '');
    my ($i, $j) = ($len_seq1, $len_seq2);
    while ($i > 0 || $j > 0) {
        if ($i > 0 && $j > 0 && $score_matrix[$i][$j] == $score_matrix[$i-1][$j-1] + (($seq1 =~ /./g)[$i-1] eq ($seq2 =~ /./g)[$j-1] ? $match : $mismatch)) {
            $align1 .= ($seq1 =~ /./g)[$i-1];
            $align2 .= ($seq2 =~ /./g)[$j-1];
            $i--; $j--;
        } elsif ($i > 0 && $score_matrix[$i][$j] == $score_matrix[$i-1][$j] + $gap) {
            $align1 .= ($seq1 =~ /./g)[$i-1];
            $align2 .= '-';
            $i--;
        } else {
            $align1 .= '-';
            $align2 .= ($seq2 =~ /./g)[$j-1];
            $j--;
        }
    }

    my $aligned_seq1 = reverse($align1);
    my $aligned_seq2 = reverse($align2);

    my $gaps = ($aligned_seq1 =~ tr/-//) + ($aligned_seq2 =~ tr/-//);
    my $score = $score_matrix[$len_seq1][$len_seq2];
    my $time_taken = tv_interval($start_time);

    return ($aligned_seq1, $aligned_seq2, $gaps, $score, $time_taken);
}

sub smith_waterman {
    my ($seq1, $seq2, $match, $mismatch, $gap) = @_;
    my $start_time = [gettimeofday];
    my $len_seq1 = length($seq1);
    my $len_seq2 = length($seq2);
    my @score_matrix;
    my ($max_score, $max_i, $max_j) = (0, 0, 0);

    for my $i (0..$len_seq1) {
        $score_matrix[$i][0] = 0;
    }
    for my $j (0..$len_seq2) {
        $score_matrix[0][$j] = 0;
    }

    for my $i (1..$len_seq1) {
        for my $j (1..$len_seq2) {
            my $match_score = $score_matrix[$i-1][$j-1] + (($seq1 =~ /./g)[$i-1] eq ($seq2 =~ /./g)[$j-1] ? $match : $mismatch);
            my $delete_score = $score_matrix[$i-1][$j] + $gap;
            my $insert_score = $score_matrix[$i][$j-1] + $gap;
            $score_matrix[$i][$j] = max(0, $match_score, $delete_score, $insert_score);
            if ($score_matrix[$i][$j] > $max_score) {
                $max_score = $score_matrix[$i][$j];
                $max_i = $i;
                $max_j = $j;
            }
        }
    }

    my ($align1, $align2) = ('', '');
    my ($i, $j) = ($max_i, $max_j);
    while ($score_matrix[$i][$j] > 0) {
        if ($i > 0 && $j > 0 && $score_matrix[$i][$j] == $score_matrix[$i-1][$j-1] + (($seq1 =~ /./g)[$i-1] eq ($seq2 =~ /./g)[$j-1] ? $match : $mismatch)) {
            $align1 .= ($seq1 =~ /./g)[$i-1];
            $align2 .= ($seq2 =~ /./g)[$j-1];
            $i--; $j--;
        } elsif ($i > 0 && $score_matrix[$i][$j] == $score_matrix[$i-1][$j] + $gap) {
            $align1 .= ($seq1 =~ /./g)[$i-1];
            $align2 .= '-';
            $i--;
        } else {
            $align1 .= '-';
            $align2 .= ($seq2 =~ /./g)[$j-1];
            $j--;
        }
    }

    my $aligned_seq1 = reverse($align1);
    my $aligned_seq2 = reverse($align2);

    my $gaps = ($aligned_seq1 =~ tr/-//) + ($aligned_seq2 =~ tr/-//);
    my $time_taken = tv_interval($start_time);

    return ($aligned_seq1, $aligned_seq2, $gaps, $max_score, $time_taken);
}

sub max {
    my ($a, $b, $c) = @_;
    return ($a > $b ? ($a > $c ? $a : $c) : ($b > $c ? $b : $c));
}

my $server_address = '127.0.0.1';
my $port = 65432;

my $socket = IO::Socket::INET->new(
    PeerHost => $server_address,
    PeerPort => $port,
    Proto => 'tcp',
) or die "Couldn't connect to server: $!\n";


my $data = '';
$socket->recv($data, 4096);
my ($seq1, $seq2) = $data =~ /seq1:(.*);seq2:(.*)/;


my ($nw_aligned_seq1, $nw_aligned_seq2, $nw_gaps, $nw_score, $nw_time_taken) = needleman_wunsch($seq1, $seq2, 1, -1, -1);


my ($sw_aligned_seq1, $sw_aligned_seq2, $sw_gaps, $sw_score, $sw_time_taken) = smith_waterman($seq1, $seq2, 1, -1, -1);


my $result = sprintf("C;Needleman;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f;Smith;Alignment1:%s;Alignment2:%s;AlignmentScore:%d;Gap:%d;ExecutionTime:%.4f",
    $nw_aligned_seq1, $nw_aligned_seq2, $nw_score, $nw_gaps, $nw_time_taken,
    $sw_aligned_seq1, $sw_aligned_seq2, $sw_score, $sw_gaps, $sw_time_taken);

$socket->send($result);
$socket->close();

print "Result sent to server.\n";