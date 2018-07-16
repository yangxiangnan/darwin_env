#!/usr/bin/perl

# This script should be pointed to a valid llvm.build folder that
# was created using the "build-llvm.pl" shell script. It will create
# a new llvm.zip file that can be checked into the respository
# at lldb/llvm.zip

use strict;
use Cwd 'abs_path';
use File::Basename;
use File::Temp qw/ tempfile tempdir /;
our $debug = 1;


sub do_command
{
	my $cmd = shift;
	my $description = @_ ? shift : "command";
	my $die_on_fail = @_ ? shift : undef;
	$debug and print "% $cmd\n";
	system ($cmd);
	if ($? == -1) 
	{
        $debug and printf ("error: %s failed to execute: $!\n", $description);
		$die_on_fail and $? and exit(1);
		return $?;
    }
    elsif ($? & 127) 
	{
        $debug and printf("error: %s child died with signal %d, %s coredump\n", 
						  $description, 
						  ($? & 127),  
						  ($? & 128) ? 'with' : 'without');
		$die_on_fail and $? and exit(1);
		return $?;
    }
    else 
	{
		my $exit = $? >> 8;
		if ($exit)
		{
			$debug and printf("error: %s child exited with value %d\n", $description, $exit);
			$die_on_fail and exit(1);
		}
		return $exit;
    }
}

if (@ARGV == 4)
{
	my $llvm_source_dir = abs_path(shift @ARGV);	# The llvm source that contains full llvm and clang sources
	my $llvm_build_dir  = abs_path(shift @ARGV);     # The llvm build directory that contains headers and 
	my $lldb_build_dir  = abs_path(shift @ARGV);     # the build directory that contains the fat libEnhancedDisassembly.dylib
	my $llvm_zip_file   = abs_path(shift @ARGV);

    printf("LLVM sources : '%s'\n", $llvm_source_dir);
    printf("LLVM build   : '%s'\n", $llvm_build_dir);
    printf("LLDB build   : '%s'\n", $lldb_build_dir);
    printf("LLVM zip file: '%s'\n", $llvm_zip_file);

	-e $llvm_build_dir or die "LLVM build directory doesn't exist: '$llvm_build_dir': $!\n";
	-l "$llvm_build_dir/llvm" || die "Couldn't find llvm symlink '$llvm_build_dir/llvm': $!\n";

	my $temp_dir = tempdir( CLEANUP => 1 );
	print "temp dir = '$temp_dir'\n";
  	my $llvm_checkpoint_dir = "$temp_dir/llvm";
	mkdir "$llvm_checkpoint_dir" or die "Couldn't make 'llvm' in '$temp_dir'\n";
	
	my @rsync_src_dst_paths =
	(
		"$llvm_source_dir/include", "$llvm_checkpoint_dir",
		"$llvm_source_dir/tools/clang/include", "$llvm_checkpoint_dir/tools/clang",
		"$llvm_build_dir/llvm/include", "$llvm_checkpoint_dir",
		"$llvm_build_dir/llvm/tools/clang/include", "$llvm_checkpoint_dir/tools/clang",
	);
	
	while (@rsync_src_dst_paths)
	{
		my $rsync_src = shift @rsync_src_dst_paths;
		my $rsync_dst = shift @rsync_src_dst_paths;
		print "rsync_src = '$rsync_src'\n";
		print "rsync_dst = '$rsync_dst'\n";
		if (-e $rsync_src)
		{
			my ($rsync_dst_file, $rsync_dst_dir) = fileparse ($rsync_dst);
			print "rsync_dst_dir = '$rsync_dst_dir'\n";
			-e $rsync_dst_dir or do_command ("mkdir -p '$rsync_dst_dir'");			
			do_command ("rsync -amvC --exclude='*.tmp' --exclude='*.txt' --exclude='*.TXT' --exclude='*.td' --exclude='\.dir' --exclude=Makefile '$rsync_src' '$rsync_dst'");
		}
	}

	do_command ("cp '$llvm_build_dir/libllvmclang.a' '$llvm_checkpoint_dir'", "Copying libllvmclang.a", 1);
	do_command ("cp '$lldb_build_dir/libEnhancedDisassembly.dylib' '$llvm_checkpoint_dir'", "Copying libEnhancedDisassembly.dylib", 1);
	do_command ("rm -rf '$llvm_zip_file'", "Removing old llvm checkpoint file '$llvm_zip_file'", 1);
	do_command ("(cd '$temp_dir' ; zip -r '$llvm_zip_file' 'llvm')", "Zipping llvm checkpoint directory '$llvm_checkpoint_dir' to '$llvm_zip_file'", 1);
}
else
{
	print "USAGE\n\tcheckpoint-llvm.pl <llvm-sources> <llvm-build> <lldb-build> <llvm-zip>\n\n";
	print "EXAMPLE\n\tcd lldb\n\t./scripts/checkpoint-llvm.pl llvm build/llvm build/BuildAndIntegration llvm.zip\n";
}
