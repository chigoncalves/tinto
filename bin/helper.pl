use 5.020;
use warnings;
use autodie;
use subs q/convert_to_lisp_style_identifier/;
use feature q/signatures/;
no warnings q/experimental::signatures/;


use Cwd q/cwd/;
use File::Spec::Functions q/catdir/;
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
  next if $dirent eq '.' || $dirent eq '..' || $dirent !~ '\.tint2rc';

  my $old_path = catdir CWD, $dir, $dirent;

  $dirent =~ s/\.tint2rc/.conf/;

  my $new_path = convert_to_lisp_style_identifier $dirent;

  $new_path = catdir CWD, $dir, $new_path;
  system "git mv $old_path $new_path";
}
