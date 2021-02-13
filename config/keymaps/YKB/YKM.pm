# Copyright (C) Andrew Suffield <asuffield@debian.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

package YKB::YKM;

use strict;
use 5.6.0;
use warnings;

use Carp;
use IO::File;
use YKB::YKB;

sub new
  {
    my $class = shift;
    my $keymap = shift;
    my $option = shift;

    my $self =
      {
       keymap => $keymap,
       option => $option,
      };

    bless $self, $class;
    return $self;
  }

sub load
  {
    my $class = shift;
    my $file = shift;
    my $strict = shift;

    my $self =
      {
       file => $file,
      };

    bless $self, $class;
    return $self->_load($strict);
  }

sub output_modifier
  {
    my $self = shift;
    my $fh = shift;
    my $modifier = shift;

    if (exists $modifier->{value})
      {
        print $fh pack 'C n', 0, $modifier->{value};
      }
    else
      {
        print $fh pack 'C n/a*', 1, $modifier->{name};
      }
  }

my %has_modifier_arg = map{$_=>1} qw/setModifiers maskModifiers toggleModifiers
                                     setStickyModifiers maskStickyModifiers toggleStickyModifiers/;
my %has_string_arg = map{$_=>1} qw/beginExtended unsetOption string event/;
my %has_list_arg = map{$_=>1} qw/setKeymap addKeymap removeKeymap/;
my %has_no_arg = map{$_=>1} qw/abortExtended clear/;

sub output_keymap
  {
    my $self = shift;
    my $fh = shift;
    my $name = shift;
    my %keymap = %{shift()};

    $name = "" if $name =~ /^__ANON__/;

    # Name
    print $fh pack 'n/a*', $name;

    print $fh pack 'n n n', map {scalar keys %$_} @keymap{'keycode', 'modifier', 'keycode name'};
    print $fh pack 'n n', map {scalar @$_} @keymap{'modifier name', 'seq'};

    foreach my $keycode (keys %{$keymap{keycode}})
      {
        if (exists $keymap{keycode}{$keycode}{value})
          {
            print $fh pack 'C n/a* n', 0, $keycode, $keymap{keycode}{$keycode}{value};
          }
        else
          {
            print $fh pack 'C n/a* n/a*', 1, $keycode, $keymap{keycode}{$keycode}{alias};
          }
      }

    foreach my $modifier (keys %{$keymap{modifier}})
      {
        my @value = @{$keymap{modifier}{$modifier}{value}};

        print $fh pack 'n/a* n', $modifier, scalar @value;

        $self->output_modifier($fh, $_) foreach @value;
      }

    foreach my $keycode (keys %{$keymap{'keycode name'}})
      {
        print $fh pack 'n/a* n/a*', $keycode, $keymap{'keycode name'}{$keycode}{name};
      }

    foreach my $modifier (@{$keymap{'modifier name'}})
      {
        my @modifier = @{$modifier->{modifier}};
        my @mask = @{$modifier->{mask}};
        print $fh pack 'n/a* n n', $modifier->{name}, scalar @modifier, scalar @mask;
        $self->output_modifier($fh, $_) foreach @modifier;
        $self->output_modifier($fh, $_) foreach @mask;
      }

    foreach my $seq (@{$keymap{seq}})
      {
        print $fh pack 'n', scalar @$seq;
        foreach my $member (@$seq)
          {
            if (exists $member->{keycode})
              {
                my @modifier = @{$member->{modifiers}};
                my @mask = @{$member->{mask}};
                print $fh pack 'C C n/a* n n', 0, $member->{direction}, $member->{keycode}, scalar @modifier, scalar @mask;
                $self->output_modifier($fh, $_) foreach @modifier;
                $self->output_modifier($fh, $_) foreach @mask;
              }
            else
              {
                print $fh pack 'C n/a*', 1, $member->{action};
                if ($has_modifier_arg{$member->{action}})
                  {
                    my @modifier = @{$member->{arg}{modifiers}};
                    print $fh pack 'C n', $member->{arg}{not}, scalar @modifier;
                    $self->output_modifier($fh, $_) foreach @modifier;
                  }
                elsif ($has_string_arg{$member->{action}})
                  {
                    print $fh pack 'n/a*', $member->{arg};
                  }
                elsif ($has_list_arg{$member->{action}})
                  {
                    my @args = @{$member->{arg}};
                    print $fh pack 'n', scalar @args;
                    print $fh pack 'n/a*', $_ foreach @args;
                  }
                elsif ($has_no_arg{$member->{action}})
                  {
                  }
                elsif ($member->{action} eq 'setOption')
                  {
                    print $fh pack 'n/a* n/a*', $member->{arg}{option}, $member->{arg}{keymap};
                  }
                else
                  {
                    die "Unhandled action '$member->{action}'";
                  }
              }
          }
      }
  }

sub output_option
  {
    my $self = shift;
    my $fh = shift;
    my $name = shift;
    my $option = shift;

    print $fh pack 'n/a*', $name;

    print $fh pack 'n', scalar keys %{$option->{keymap}};

    foreach my $keymap (keys %{$option->{keymap}})
      {
        if (ref $option->{keymap}{$keymap} eq 'HASH')
          {
            # Embedded keymap
            print $fh pack 'C', 0;
            $self->output_keymap($fh, $keymap, $option->{keymap}{$keymap}) ;
          }
        else
          {
            # Reference by name
            print $fh pack 'C n/a*', 1, $option->{keymap}{$keymap};
          }
      }
  }

sub output
  {
    my $self = shift;
    my $output = shift;

    my $fh = new IO::File $output, "w";
    unless ($fh)
      {
        print STDERR "Failed to open $output: $!\n";
        return 0;
      }

    # Header: null byte, "YKM", version number
    print $fh pack 'C a* C', 0, "YKM", 0;

    # Count of keymaps and options
    print $fh pack 'n n', scalar keys %{$self->{keymap}}, scalar keys %{$self->{option}};

    $self->output_keymap($fh, $_, $self->{keymap}{$_}) foreach keys %{$self->{keymap}};
    $self->output_option($fh, $_, $self->{option}{$_}) foreach keys %{$self->{option}};

    return 1;
  }

sub safe_read
  {
    my $fh = shift;
    my $len = shift;
    my $data = "";
    while ($len > 0)
      {
        my $rc = $fh->read($data, $len, length $data);
        confess "Unexpected EOF (wanted to read another $len bytes)" if $rc == 0;
        die "Read failed: $!" unless defined $rc;
        $len -= $rc;
      }
    return $data;
  }

sub load_string
  {
    my $self = shift;
    my $fh = shift;

    my $len = unpack('n', safe_read($fh, 2));
    return unpack('a*', safe_read($fh, $len));
  }

sub load_modifier
  {
    my $self = shift;
    my $fh = shift;

    my $type = unpack 'C', safe_read($fh, 1);
    if ($type == 0)
      {
        my $value = unpack 'n', safe_read($fh, 2);
        return {value => $value};
      }
    elsif ($type == 1)
      {
        my $name = $self->load_string($fh);
        return {name => $name};
      }
    else
      {
        die "Invalid modifier type $type";
      }
  }

sub load_keymap
  {
    my $self = shift;
    my $fh = shift;

    my $errors = 0;
    my $warnings = 0;

    my $keymap_name = $self->load_string($fh);

    my ($keycode_count, $modifier_count, $keycode_name_count, $modifier_name_count, $seq_count) = unpack('nnnnn', safe_read($fh, 10));

    my %keycode;
    my %modifier;
    my %keycode_name;
    my @modifier_name;
    my @seq;

    foreach (1..$keycode_count)
      {
        my $type = unpack 'C', safe_read($fh, 1);
        my $name = $self->load_string($fh);

        if (exists $keycode{$name})
          {
            print STDERR "Keycode $name redefined (in keymap $keymap_name)\n";
            $warnings++;
          }

        if ($type == 0)
          {
            my $value = unpack 'n', safe_read($fh, 2);
            $keycode{$name} = {value => $value};
          }
        elsif ($type == 1)
          {
            my $alias = $self->load_string($fh);
            $keycode{$name} = {alias => $alias};
          }
        else
          {
            die "Invalid keycode type $type";
          }
      }

    foreach (1..$modifier_count)
      {
        my $name = $self->load_string($fh);
        my $value_count = unpack 'n', safe_read($fh, 2);

        if (exists $modifier{$name})
          {
            print STDERR "Modifier $name redefined (in keymap $keymap_name)\n";
            $warnings++;
          }

        my @value = map {$self->load_modifier($fh)} 1..$value_count;

        $modifier{$name} = {value => \@value};
      }

    foreach (1..$keycode_name_count)
      {
        my $keycode = $self->load_string($fh);
        my $name = $self->load_string($fh);

        if (exists $keycode_name{$keycode})
          {
            print STDERR "Keycode name for $name redefined (in keymap $keymap_name)\n";
            $warnings++;
          }

        $keycode_name{$keycode} = {name => $name};
      }

    foreach (1..$modifier_name_count)
      {
        my $name = $self->load_string($fh);
        my ($modifier_count, $mask_count) = unpack 'nn', safe_read($fh, 4);

        my @modifier = map {$self->load_modifier($fh)} 1..$modifier_count;
        my @mask = map {$self->load_modifier($fh)} 1..$mask_count;

        push @modifier_name, {name => $name,
                              modifier => \@modifier,
                              mask => \@mask,
                             };
      }

    foreach (1..$seq_count)
      {
        my $count = unpack 'n', safe_read($fh, 2);
        my @member;
        foreach (1..$count)
          {
            my $type = unpack 'C', safe_read($fh, 1);
            if ($type == 0)
              {
                my $direction = unpack 'C', safe_read($fh, 1);
                my $keycode = $self->load_string($fh);
                my ($modifier_count, $mask_count) = unpack 'nn', safe_read($fh, 4);

                my @modifier = map {$self->load_modifier($fh)} 1..$modifier_count;
                my @mask = map {$self->load_modifier($fh)} 1..$mask_count;

                push @member, {direction => $direction, keycode => $keycode, modifiers => \@modifier, mask => \@mask};
              }
            elsif ($type == 1)
              {
                my $action = $self->load_string($fh);
                my $arg;

                if ($has_modifier_arg{$action})
                  {
                    my ($not, $modifier_count) = unpack 'Cn', safe_read($fh, 3);

                    my @modifiers = map {$self->load_modifier($fh)} 1..$modifier_count;

                    $arg = {not => $not, modifiers => \@modifiers};
                  }
                elsif ($has_string_arg{$action})
                  {
                    $arg = $self->load_string($fh);
                  }
                elsif ($has_list_arg{$action})
                  {
                    my $arg_count = unpack 'n', safe_read($fh, 2);
                    $arg = [map {$self->load_string($fh)} 1..$arg_count];
                  }
                elsif ($has_no_arg{$action})
                  {
                  }
                elsif ($action eq 'setOption')
                  {
                    my $option = $self->load_string($fh);
                    my $keymap = $self->load_string($fh);
                    $arg = {option => $option, keymap => $keymap};
                  }
                else
                  {
                    die "Unhandled action '$action'";
                  }
                push @member, {action => $action, $arg ? (arg => $arg) : ()};
              }
            else
              {
                die "Invalid member type in seq";
              }
          }
        push @seq, \@member;
      }

    return {name => $keymap_name,
            keycode => \%keycode,
            modifier => \%modifier,
            'keycode name' => \%keycode_name,
            'modifier name' => \@modifier_name,
            seq => \@seq,
           }, $errors, $warnings;
  }

sub load_option
  {
    my $self = shift;
    my $fh = shift;

    my $errors = 0;
    my $warnings = 0;

    my $name = $self->load_string($fh);
    my $keymap_count = unpack 'n', safe_read($fh, 2);
    my %keymap;

    foreach (1..$keymap_count)
      {
        my $type = unpack 'C', safe_read($fh, 1);
        if ($type == 0)
          {
            my ($keymap, $new_errors, $new_warnings) = $self->load_keymap($fh);
            $errors += $new_errors;
            $warnings += $new_warnings;
            next unless defined $keymap;

            my $name = $keymap->{name};
            if (exists $keymap{$name})
              {
                print STDERR "Duplicate keymap name '$name'\n";
                $warnings++;
              }
            $keymap{$name} = $keymap;
          }
        elsif ($type == 1)
          {
            my $keymap_name = $self->load_string($fh);
            $keymap{$keymap_name} = $keymap_name;
          }
        else
          {
            die "Invalid option keymap type $type";
          }
      }

    return {name => $name,
            keymap => \%keymap,
           }, $errors, $warnings;
  }

sub _load
  {
    my $self = shift;
    my $strict = shift;

    my $errors = 0;
    my $warnings = 0;

    my $fh = new IO::File $self->{file}, "r";
    unless ($fh)
      {
        print STDERR "Failed to open $self->{file}: $!\n";
        return undef;
      }

    my $header = safe_read($fh, 4);
    unless ($header eq pack 'C a*', 0, "YKM")
      {
        print STDERR "Invalid YKM header\n";
        return undef;
      }

    my $version = unpack('C', safe_read($fh, 1));
    unless ($version == 0)
      {
        print STDERR "YKM format version $version is not supported\n";
        return undef;
      }

    my ($keymap_count, $option_count) = unpack('n n', safe_read($fh, 4));
    my @keymaps;
    my @options;

    $self->{keymap} = {};
    $self->{option} = {};

    foreach (1..$keymap_count)
      {
        my ($keymap, $new_errors, $new_warnings) = $self->load_keymap($fh);
        $errors += $new_errors;
        $warnings += $new_warnings;
        next unless defined $keymap;

        my $name = $keymap->{name};
        if (exists $self->{keymap}{$name})
          {
            print STDERR "Duplicate keymap name '$name'\n";
            $warnings++;
          }
        $self->{keymap}{$name} = $keymap;
      }

    foreach (1..$option_count)
      {
        my ($option, $new_errors, $new_warnings) = $self->load_option($fh);
        $errors += $new_errors;
        $warnings += $new_warnings;
        next unless defined $option;

        my $name = $option->{name};
        if (exists $self->{option}{$name})
          {
            print STDERR "Duplicate option name '$name'\n";
            $warnings++;
          }
        $self->{option}{$name} = $option;
      }

    return undef if $errors;
    return undef if $warnings and $strict;
    return $self;
  }

sub ykb
  {
    my $self = shift;

    my $ykb = new YKB::YKB $self->{keymap}, $self->{option};
    return $ykb;
  }

1;
