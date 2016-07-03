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

sub trim_2 {
  my $str = shift;
  $str =~ s/^\s*|\s*$//g;
  $str =~ s/;//g;

  return $str;
}

sub proc_arg ($arg) {
  $arg = trim_2 $arg;
  my %arg = (type => '', name => '', ptr => '', array => '');
  if ($arg =~ /(?<type>\w+)\s*\(?<name>\w+)/) {

  }
}

while (my $line = <DATA>) {
  $line = trim_2 $line;
  # say $line;
  if ($line =~ /^(?<ret_type>\w+)\s*\**(?<ptr_opr>\*)?\s*(?<name>\w+)\s*\(
		(?<arg_list>(.*))?
	       \s*\)/xg) {
    say sprintf ('%s -> %s', $line, (defined $+{arg_list} ? $+{arg_list} : ''));

  }
  elsif ($line =~ /^(?<type>\w+)\s*(?<ptr_opr>\*)?\s*(?<name>\w+)
		(?<is_array>\s*\[\s*(\w|\d)*\s*\]\s*)?$/x) {
    my ($type, $name) = ($+{type}, $+{name});
    say sprintf ('%s : %s%s%s', $name, $type,
		 (defined $+{ptr_opr} ? '*' : ''),
		 (defined $+{is_array} ? '[]' : ''));

  }

}


__DATA__
void tinto_usage (void)
int** foo_a ()
int **foo_b()
  int *foo_c ()
  int* foo_d ()
  void qux (int bar)
  void free (void *ptr)
  void* realloc (void *ptr, size_t size, int foo)
  void* baz (int a, char* c, double a)
int f (person_t p, int chr, char b, foo f, bar_t bar, foo h)
