#!/usr/bin/perl

use POSIX;
use Getopt::Std;
use File::Compare;
use Fcntl;

# enable use of -p and -s flags
my %options=();
getopts("ps", \%options);


my @scene_names = ("random-10000", "random-50000", "corner-10000", "corner-50000", "sparse-50000", "sparse-200000");
my @particle_nums = (10000, 50000, 10000, 50000, 10000, 50000, 50000, 200000);
my @space_sizes = (100.0, 500.0, 100.0, 500.0, 5.0, 20.0);
my @iterations = (5, 5, 5, 5, 50, 50);

my @scene_percent = (.10, .15, .15, .225, .15, .225);
my @target_total = (9.0, 9.0, 9.0, 9.5, 5.5, 6.0);
my @target_tree = (2.5, 2.9, 1.7, 2.6, 3.5, 3.5);

my @correct = (1,1,1,1,1,1);

my %your_par_build_times;
my %your_par_total_times;
my %your_par1_build_times;
my %ref_total_times;

`mkdir -p logs`;
`rm -rf logs/*`;

print "\n";
print ("--------------\n");
my $hostname = `hostname`;
print ("Running tests on $hostname\n");
print ("--------------\n");

# if (index(lc($hostname),"ghc") == -1) {
#     $render_ref = "render_ref_latedays";
# }

$length = @scene_names;

if (defined $options{p}) {
    for (my $i=0; $i < $length; $i++) {
            print ("\n**********************  Scene : @scene_names[$i]  **********************\n\n");
            $iter = @iterations[$i];
            print ("$iter Iterations \n");

            process_implementation(@scene_names[$i], "Parallel", "-par", "./nbody-release", $i);
            print "\n";
            process_implementation(@scene_names[$i], "Parallel with One Thread", "-par", "OMP_NUM_THREADS=1 ./nbody-release", $i);
            print "\n";
            process_implementation(@scene_names[$i], "Reference Sequential", "-seq", "./nbody-ref", $i);
    }
    print_summary();
}

if (defined $options{s}) {
    $all_correct = 1;
    for (my $i=0; $i < $length; $i++) {
        $scene =  @scene_names[$i];
        print ("\n**********************  Scene : @scene_names[$i]  **********************\n\n");
        $iter = @iterations[$i];
        print ("$iter Iterations \n");
         
        process_implementation(@scene_names[$i], "Sequential", "-seq", "./nbody-release", $i);
        print "\n";
        if (!@correct[$i]){
            $all_correct = 0;
        }
    }
    if ($all_correct != 1){
        print "\nCORRECTNESS FAILED - SCORE: 0 pts\n";
    }
    else {
        print "\nCORRECT!!  - SCORE: 10 pts\n";
    }
}

sub process_implementation {
        $scene = $_[0]; 
        $implementation_type = $_[1]; 
        $type_flag = $_[2]; 
        $executable = $_[3]; 
        $i = $_[4]; 
        $iter =  @iterations[$i];
        $num_particles = $particle_nums[$i];
        $space_size = $space_sizes[$i];
        $init_file = "./src/benchmark-files/".$scene."-init.txt";
        $output_file = "./logs/".$scene.$type_flag.".txt";
        if ($executable eq "./nbody-ref"){
            $output_file = "./logs/".$scene.$type_flag."-ref.txt";
        }
        $ref_file = "./src/benchmark-files/".$scene."-ref.txt";
        print ("$implementation_type Implementation:\n");
        my @sys_stdout = system ("$executable $type_flag -n $num_particles -i $iter -in $init_file -s $space_size  -o $output_file > ./logs/correctness_${scene}.log");
        my $return_value  = $?;
        if ($return_value != 0) {
            print ("Nbody Release exited with errors.!\n");
        }
        
        # todo: check output here
        if (compare_files($output_file, $ref_file) == 0) {
            print ("\nCorrectness passed!\n\n");
        }
        else {
            print ("Correctness failed ... Check ./logs/correctness_${scene}.log and compare $output_file with reference $ref_file\n");
            @correct[$i] = 0;
        }
        my $your_total_time = ` grep "TOTAL TIME:" ./logs/correctness_${scene}.log`;
        my $your_build_time = `grep "total tree construction time:" ./logs/correctness_${scene}.log`;

        chomp($your_total_time);
        chomp($your_build_time);
        $your_total_time =~ s/^[^0-9]*//;
        $your_total_time =~ s/ s.*//;
        $your_build_time =~ s/^[^0-9]*//;
        $your_build_time =~ s/ s.*//;
        print (sprintf ("%-40s %-40s \n", "Total Time :", $your_total_time));
        print (sprintf ("%-40s %-40s \n", "Total Build Tree Time :", $your_build_time));
        
        if ($implementation_type eq "Parallel") {
            $your_par_build_times{$scene} = $your_build_time;
            $your_par_total_times{$scene} = $your_total_time;
        } 
        if ($implementation_type eq "Parallel with One Thread") { 
            $your_par1_build_times{$scene} = $your_build_time;
        }
        if ($implementation_type eq "Reference Sequential") { 
            $ref_total_times{$scene} = $your_total_time;
        }
}

# Correctness Checking
sub compare_files {
  $result = $_[0]; 
  $ref = $_[1]; 

  open(my $fh1, '<:encoding(UTF-8)', $result)
    or die "Could not open file '$result' $!";
  open(my $fh2, '<:encoding(UTF-8)', $ref)
    or die "Could not open file '$ref' $!";

  #compare file length
  for ($count1=0; <$fh1>; $count1++) { }
  for ($count2=0; <$fh2>; $count2++) { }

  if ($count1 != $count2) {
      print "ERROR -- Mismatch: number of particles is $count1, should be $count2\n";
      return 1;
  }

  #return to top of file
  seek $fh1, 0, 0;
  seek $fh2, 0, 0;

  #compare values
  $line_num = 0;
  while (my $line1 = <$fh1>) {
    my $line2 = <$fh2>;
    $line_num = $line_num + 1;

    chomp $row;
    my @words1 = split / /, $line1;
    my @words2 = split / /, $line2;
    for (my $i=0; $i < 5; $i++)
    {
        my $error = abs(@words1[$i] - @words2[$i]);
        if ($error > 0.05){ # equivalent to 1e-2f
           $val1  =  @words1[$i];
           $val2  =  @words2[$i];
           print "ERROR -- Mismatch: found correctness error at line $line_num index $i, has value $val1, should be $val2 (with delta up to .05)\n";
           return 1;
        }
    }
  }

  return 0;
}

sub print_summary {
    print "\n";
    print ("------------\n");
    print ("Speedup table:\n");
    print ("------------\n");

    my $header = sprintf ("| %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n", "Scene Name", "Build Speedup", "Target", "Total Speedup", "Target", "Total Speedup",);
    my $dashes = $header;
    $dashes =~ s/./-/g;
    print $dashes;
    print $header;
    print $dashes;

    my $total_tree_score = 0;
    my $total_total_score = 0;

    my $total_correct = 1;
    for (my $i=0; $i < $length; $i++) {
        
        my $scene = @scene_names[$i];
        my $tree_score;
        my $total_score;
        
        my $tree_num = $your_par1_build_times{$scene}/$your_par_build_times{$scene};
        my $tree_speedup .= sprintf("%.6f", $tree_num);
        my $total_num = $ref_total_times{$scene}/$your_par_total_times{$scene};
        my $total_speedup .= sprintf("%.6f", $total_num);

        if (! @correct[$i]) {
            $total_speedup .= " (F)";
            $total_correct = 0;
            $tree_score = 0;
            $total_score = 0;
        } else {
            if ($total_num >= $target_total[$i]) {
                $total_score = 20 * @scene_percent[$i];
            } else {
                if ($total_num  <= 0.0000001) {
                    $total_score = 0;
                }
                else {
                    $total_score = (20 * @scene_percent[$i]) * ($total_num/$target_total[$i]);
                }
            }
            if ($tree_num  >= $target_tree[$i]) {
                $tree_score = 10 * @scene_percent[$i];
            } else {
                if ($tree_num  <= 0.0000001) {
                    $tree_score = 0;
                } else {
                   $tree_score = (10 * @scene_percent[$i]) * ($tree_num/$target_tree[$i]);

                }
            }
        }

        printf ("| %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n", "$scene", "$tree_speedup", "@target_tree[$i]", "$total_speedup", "@target_total[$i]",  "$total_speedup" );
        $total_tree_score += $tree_score;
        $total_total_score += $total_score;
    }
    $total = sprintf("%.3f",$total_tree_score+$total_total_score);
    $totalt = sprintf("%.3f",$total_tree_score);
    $totals = sprintf("%.3f",$total_total_score);
    printf ("| %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n", "SCORE", "$totalt", "",  "$totals", "",  "($total)");

    if ($total_correct != 1) {
        print "\nCORRECTNESS FAILED - SCORE: 0\n";
    }
}
