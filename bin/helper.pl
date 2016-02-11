use 5.020;
use warnings;
use autodie;
use feature q/signatures/;
no warnings q/experimental::signatures/;
use subs qw(
  convert_to_lisp_style_identifier
  proc_conf_file
  str_trim
  convert_color_alpha
);


use Cwd q/cwd/;
use File::Spec::Functions q/catdir/;
use POSIX q/floor/;

use constant CWD => cwd;

my $dir = 'res/config-sample';

die "`$dir' does not exist!\n" unless -d $dir;

sub convert_to_lisp_style_identifier ($identifier) {
  $identifier = lc $identifier;
  $identifier =~ s/\s/-/g;
  $identifier =~ s/_/-/g;
  $identifier =~ s/-+/-/g;
  $identifier =~ s/(^(-|\d)|-$)//g;

  return $identifier;
}

opendir (my $DH, $dir);
while (my $dirent = readdir $DH) {
  next if $dirent eq '.' || $dirent eq '..' || $dirent !~ /\.conf$/;

  my $old_path = catdir CWD, $dir, $dirent;
  my $new_file = sprintf "%s.bkp", $old_path;
  proc_conf_file $old_path, $new_file;

  rename $new_file, $old_path;
}

# sub f {
#   my $old_path = catdir CWD, $dir, $dirent;

#   $dirent =~ s/\.tint2rc/.conf/;

#   my $new_path = convert_to_lisp_style_identifier $dirent;

#   $new_path = catdir CWD, $dir, $new_path;
#   system "git mv $old_path $new_path";
# }

sub proc_conf_file ($filename, $new_file) {
  open (my $IN, '<', $filename);
  open (my $OUT, '>', $new_file);

  while (my $line = <$IN>) {
    chomp $line;

    if ($line !~ /^#/ && $line =~ /color/) {
      if ($line =~ /(?<keyword>\w+)\s+=\s+(?<color>#[abcdef0-9]{6})\s+(?<alpha>[0-9]{2,})/i) {
	my $color = lc $+{color};
	my $value = convert_color_alpha $+{alpha};
	$value = sprintf "%x%x", floor ($value / 16), $value % 16;
	say {$OUT} "$+{keyword} = $color", ($value eq 'ff' ? '' : $value);

      }
    }
    else {
      say {$OUT} $line;
    }
  }

  close $IN;
  close $OUT;
}

sub str_trim ($str) {
  $str =~ s/\s+//g;

  return $str;
}

sub convert_color_alpha ($color) {
  return 255 * $color / 100;
}
