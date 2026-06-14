#! /usr/bin/env perl

use strict;
use warnings;

use Cwd qw( cwd getcwd realpath );
use Getopt::Long;
use File::Temp qw( tempdir );

my $arg = 0;
my @filelist = [];
my $format = "";
my $check = "";

sub usage
{
    print "Usage: $0 [OPTIONS] [filelist]\n\n";
    print "OPTIONS:\n";
    print "\t--check                Do not reformat, just emit errors where formating is wrong\n";
    print "\t--help                 Show this help text\n";
    print "\n";
    exit 1;
}

sub get_files
{
    my $file = "";
    foreach $file (@ARGV) {
        unless (-f $file) {
            print "$0 expects a list of files\n";
            print "$file does not exist\n";
            exit 1;
        }
    }
    @filelist = @ARGV;
    print "Formatting @filelist\n";
}

sub get_omb_c_files
{
    my @c_files = "";

    # Show current branch, get all c header etc files baby
    my $commit = `git rev-parse --short HEAD`;
    my @all_files = `git ls-tree --full-tree -r --name-only $commit`;
    @c_files = grep m/\.(c|cpp|cu|h)$/, @all_files;


    @filelist = @c_files;
    print "Formatting @filelist";
}


sub check_format_method
{
    if (`which clang-format` eq "") {
        print "clang-format preferred but not found\n";
        print "Please add clang-format to your path\n";
        exit 1;
    } else {
        my $v = `clang-format --version | head -1 | cut -f3 -d' ' | xargs echo -n`;
        print "clang-format version $v found, using clang-format for formatting\n";
        $format = "clang-format";
    }
}


GetOptions(
    "check" => \$check,
    "help" => \&usage,
) or die "unable to parse options, stopped";


# check for clang-format
check_format_method();
print("\n");

if (scalar(@ARGV) != 0) {
    get_files();
    print("\n");
} else {
    get_omb_c_files();
}
chomp(@filelist);

my $command = "clang-format -style=file";
if ($check) {
    $command .= " -dry-run -Werror";
} else {
    $command .= " -i"
}
my $res = system("$command @filelist");
if ($check and $res) {
    print "Formatting errors found, please use this script to reformat your code\n";
    exit 1;
} elsif ($check) {
    print "No formatting errors found.\n"
}
exit 0
