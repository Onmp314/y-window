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

package YKB::YKB;

use strict;
use 5.6.0;
use warnings;

use YKB::YKM;
use IO::File;
use Storable qw/dclone/;
use YKB::YKBGrammar;

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

my $parser = new YKB::YKBGrammar;

sub _parse_file
  {
    my $filename = shift;
    my $fh = new IO::File $filename, "r";
    unless ($fh)
      {
        print STDERR "Failed to open $filename: $!\n";
        return undef;
      }

    local $/ = undef;
    my $data = <$fh>;
    $fh->close;

    $::YKB_file = $filename;
    return $parser->file($data);
  }

sub expand_block_include
  {
    my $self = shift;
    my $block = shift;
    my $blocks = shift;

    my $errors = 0;

    my $statements = $block->{statements};

    for (my $i = 0; $i < @$statements; $i++)
      {
        my $statement = $statements->[$i];
        # Splice out comments
        unless (ref $statement)
          {
            splice @$statements, $i, 1;
            redo;
          }
        next unless $statement->{include};

        my $file = $statement->{file} || die "invalid statement";
        my $line = $statement->{line} || 0;
        my $type = $statement->{type};
        my $name = $statement->{name};
        die "$file:$line: undefined include type" unless defined $type;
        die "$file:$line: undefined include name" unless defined $type;

        unless (exists $blocks->{$type}{$name})
          {
            print STDERR "$file:$line: unknown $type block '$name'\n";
            $errors++;
            next;
          }

        my $target = $blocks->{$type}{$name};
        my $included = $target->{statements};

        # Splice the included statements in place of the include statement
        splice @$statements, $i, 1, @$included;
        redo;
      }

    return $errors;
  }

sub anon_keymap
  {
    my $self = shift;

    return "__ANON__" . $self->{anonymous_keymaps}++;
  }

sub parse_modifier
  {
    my $self = shift;
    my $mask = shift;

    my @value;

    foreach my $modifier (@$mask)
      {
        my $file = $modifier->{file};
        my $line = $modifier->{line};
        if (exists $modifier->{index})
          {
            push @value, {value => 1 << $modifier->{index}};
          }
        elsif (exists $modifier->{name})
          {
            my $name = $modifier->{name};
            push @value, {name => $name};
          }
        else
          {
            die "$file:$line: invalid modifier mask";
          }
      }

    return \@value;
  }

sub parse_options
  {
    my $self = shift;
    my $keymaps = shift;
    my $parent = shift;

    my $options = {};

    return $options;
  }

sub parse_sequence
  {
    my $self = shift;
    my $seq = shift;
    my $file = shift;
    my $line = shift;

    my @result;

    foreach my $member (@$seq)
      {
        if (exists $member->{keycode})
          {
            my $modifiers = exists $member->{modifiers} ? $self->parse_modifier($member->{modifiers}) : [];
            my $mask = exists $member->{mask} ? $self->parse_modifier($member->{mask}) : [{value => (1 << 16) - 1}];

            if (not defined $member->{direction})
              {
                push @result, {direction => 1, keycode => $member->{keycode}, modifiers => $modifiers, mask => $mask};
              }
            elsif ($member->{direction} eq '^')
              {
                push @result, {direction => 0, keycode => $member->{keycode}, modifiers => $modifiers, mask => $mask};
              }
            elsif ($member->{direction} eq '*')
              {
                push @result, {direction => 1, keycode => $member->{keycode}, modifiers => $modifiers, mask => $mask};
                push @result, {direction => 0, keycode => $member->{keycode}, modifiers => dclone($modifiers), mask => dclone($mask)};
              }
          }
        elsif (exists $member->{action})
          {
            my $arg = $member->{arg};
            if (defined $arg and ref $arg eq 'HASH')
              {
                if (exists $arg->{modifiers})
                  {
                    $arg->{modifiers} = $self->parse_modifier($arg->{modifiers});
                    $arg->{not} = defined $arg->{not};
                  }
              }
            push @result, {action => $member->{action}, arg => $member->{arg}};
          }
        else
          {
            die "$file:$line: invalid seq member";
          }
      }

    return \@result;
  }

sub parse_keymap
  {
    my $self = shift;
    my $block = shift;

    my $warnings = 0;
    my $errors = 0;

    my $keymap = {name => $block->{name} || $self->anon_keymap,
                  keycode => {},
                  modifier => {},
                  'keycode name' => {},
                  'modifier name' => [],
                  seq => [],
                  file => $block->{file},
                  line => $block->{line},
                 };

    my $statements = $block->{statements};

    foreach my $statement (@$statements)
      {
        my $file = $statement->{file} || die "invalid statement";
        my $line = $statement->{line} || 0;

        if ($statement->{type} eq 'keycode')
          {
            my $name = $statement->{name};

            if (exists $keymap->{keycode}{$name})
              {
                my $oldfile = $keymap->{keycode}{$name}{file};
                my $oldline = $keymap->{keycode}{$name}{line};
                print STDERR "$file:$line: keycode '$name' redefined\n";
                print STDERR "$oldfile:$oldline: (previous definition here)\n";
                $warnings++;
              }

            if (exists $statement->{value})
              {
                my $value = $statement->{value};
                $keymap->{keycode}{$name} = {value => $value,
                                             file => $file,
                                             line => $line,
                                            };
              }
            elsif (exists $statement->{alias})
              {
                my $alias = $statement->{alias};

                $keymap->{keycode}{$name} = {alias => $alias,
                                             file => $file,
                                             line => $line,
                                            };
              }
            else
              {
                die "$file:$line: incomplete keycode statement";
              }
          }
        elsif ($statement->{type} eq 'modifier')
          {
            my $name = $statement->{name};
            my $mask = $statement->{value};

            if (exists $keymap->{modifier}{$name})
              {
                my $oldfile = $keymap->{modifier}{$name}{file};
                my $oldline = $keymap->{modifier}{$name}{line};
                print STDERR "$file:$line: modifier '$name' redefined\n";
                print STDERR "$oldfile:$oldline: (previous definition here)\n";
                $warnings++;
              }

            my $value = $self->parse_modifier($mask);
            unless (defined $value)
              {
                $errors++;
                next;
              }

            $keymap->{modifier}{$name} = {value => $value,
                                          file => $file,
                                          line => $line,
                                         };
          }
        elsif ($statement->{type} eq 'keycode name')
          {
            my $name = $statement->{name};
            my $desc = $statement->{desc};

            if (exists $keymap->{'keycode name'}{$name})
              {
                my $oldfile = $keymap->{'keycode name'}{$name}{file};
                my $oldline = $keymap->{'keycode name'}{$name}{line};
                print STDERR "$file:$line: name for keycode '$name' redefined\n";
                print STDERR "$oldfile:$oldline: (previous definition here)\n";
                $warnings++;
              }

            $keymap->{'keycode name'}{$name} = {name => $desc,
                                                file => $file,
                                                line => $line,
                                               };
          }
        elsif ($statement->{type} eq 'modifier name')
          {
            my $modifier = $statement->{modifier};
            my $mask = $statement->{mask};
            my $desc = $statement->{desc};

            # If no mask was specified, set the mask to be the same as
            # the modifier - "anything with precisely these set". This
            # is the "natural" behaviour.
            $mask = $modifier unless defined $mask;

            my $modifier_value = $self->parse_modifier($modifier, $keymap);
            unless (defined $modifier_value)
              {
                $errors++;
                next;
              }

            my $mask_value = $self->parse_modifier($mask, $keymap);
            unless (defined $mask_value)
              {
                $errors++;
                next;
              }

            push @{$keymap->{'modifier name'}}, {modifier => $modifier_value,
                                                 mask => $mask_value,
                                                 name => $desc,
                                                 file => $file,
                                                 line => $line,
                                                };
          }
        elsif ($statement->{type} eq 'seq')
          {
            my $seq = $self->parse_sequence($statement->{sequence}, $keymap, $statement->{file}, $statement->{line});
            unless (defined $seq)
              {
                $errors++;
                next;
              }

            push @{$keymap->{seq}}, $seq;
          }
        else
          {
            die "$file:$line: unrecognised statement type $statement->{type}";
          }
      }

    return ($keymap, $errors, $warnings);
  }

sub _load
  {
    my $self = shift;
    my $strict = shift;

    my $parsed = _parse_file($self->{file});
    return undef unless defined $parsed;

    # Now, first we study the blocks, making a note of all the
    # keymaps, options, and named blocks

    my @keymaps;
    my @options;
    my @blocks;
    my $errors = 0;
    my $warnings = 0;

    my %blocks = (keymap => {},
                  keycode => {},
                  modifier => {},
                  desc => {},
                  seq => {},
                  option => {},
                 );

    my $seen_default = undef;

    foreach my $block (@$parsed)
      {
        # Skip comments
        next unless ref $block;

        my $file = $block->{file} || die "invalid block";
        my $line = $block->{line} || die "invalid block";
        my $type = $block->{type};
        die "$file:$line: undefined block type" unless defined $type;
        die "$file:$line: unrecognised block type '$block->{type}'" unless exists $blocks{$type};

        my $name = $block->{name};

        if (defined $name)
          {
            # Duplicates are discarded out of hand
            if (exists $blocks{$type}{$name})
              {
                my $oldfile = $blocks{$type}{$name}{file};
                my $oldline = $blocks{$type}{$name}{line};
                print STDERR "$file:$line: $type block '$name' redefined\n";
                print STDERR "$oldfile:$oldline: (previously defined here)\n";
                $errors++;
                next;
              }

            $blocks{$type}{$name} = $block;
          }
        else
          {
            # Only keymaps can be anonymous
            if ($type ne 'keymap')
              {
                print STDERR "$file:$line: useless definition of an anonymous $type block\n";
                $errors++;
                next;
              }
          }

        if ($type eq 'keymap')
          {
            push @keymaps, $block;
            if ($block->{default})
              {
                if ($seen_default)
                  {
                    my $oldfile = $seen_default->{file};
                    my $oldline = $seen_default->{line};
                    print STDERR "$file:$line: multiple default keymaps are not allowed\n";
                    print STDERR "$oldfile:$oldline: (first default keymap here)\n";
                    $errors++;
                    next;
                  }
                $seen_default = {file => $file,
                                 line => $line,
                                };
              }
          }

        push @options, $block if $type eq 'option';
      }

    # Don't need this any more
    undef $parsed;

    unless (scalar @keymaps)
      {
        print STDERR "$self->{file}:0: at least one keymap is required\n";
        return undef;
      }

    # Now we expand every block include directive, in a recursive
    # descent fashion (note that include statements which can't be
    # reached from a keymap are never expanded). We also eliminate
    # comments here.

    $errors += $self->expand_block_include($_, \%blocks) foreach @keymaps;

    # And the same for optional keymaps
    foreach my $option (@options)
      {
        my $keymaps = $option->{keymaps};
        $errors += $self->expand_block_include($_, \%blocks) foreach grep {exists $_->{statements}} @$keymaps;
      }

    # Don't need these any more
    %blocks = ();

    # Now we actually process the statements

    $self->{keymap} = {};
    $self->{anonymous_keymaps} = 0;

    foreach my $block (@keymaps)
      {
        my $file = $block->{file};
        my $line = $block->{line};
        my $name = $block->{name};
        if (defined $name and exists $self->{keymap}{$name})
          {
            my $oldfile = $self->{keymap}{$name}{file};
            my $oldline = $self->{keymap}{$name}{line};
            print STDERR "$file:$line: keymap '$name' redefined\n";
            print STDERR "$oldfile:$oldline: (previous definition here)\n";
            $errors++;
            next;
          }

        my ($keymap, $new_errors, $new_warnings) = $self->parse_keymap($block);
        $errors += $new_errors;
        $warnings += $new_warnings;
        next unless defined $keymap;

        $self->{keymap}{$keymap->{name}} = $keymap;

        # If we don't see an explicit 'default' keyword, then the
        # first one is the default
        $self->{default_keymap} = $keymap unless $seen_default or $self->{default_keymap};
        $self->{default_keymap} = $keymap if $block->{default};
      }

    $self->{option} = {};

    foreach my $option (@options)
      {
        my $name = $option->{name};
        if (exists $self->{option}{$name})
          {
            my $file = $option->{file};
            my $line = $option->{line};
            my $oldfile = $self->{option}{$name}{file};
            my $oldline = $self->{option}{$name}{line};
            print STDERR "$file:$line: option '$name' redefined\n";
            print STDERR "$oldfile:$oldline: (previous definition here)\n";
            $errors++;
            next;
          }

        my $keymaps = {};
        $self->{option}{$name} = {keymap => $keymaps,
                                  file => $option->{file},
                                  line => $option->{line},
                                 };

        foreach my $block (@{$option->{keymaps}})
          {
            my $file = $block->{file};
            my $line = $block->{line};
            my $name = $block->{name};

            if (exists $keymaps->{$name})
              {
                my $oldfile = $keymaps->{$name}{file};
                my $oldline = $keymaps->{$name}{line};
                print STDERR "$file:$line: option keymap '$name' redefined\n";
                print STDERR "$oldfile:$oldline: (previous definition here)\n";
                $errors++;
                next;
              }

            my $keymap;

            if (exists $block->{statements})
              {
                my $new_errors;
                ($keymap, $new_errors) = $self->parse_keymap($block);
                $errors += $new_errors;
                next unless defined $keymap;
              }
            else
              {
                unless (exists $self->{keymap}{$name})
                  {
                    print STDERR "$file:$line: undefined keymap '$name'\n";
                    $errors++;
                    next;
                  }

                $keymap = $name;
              }

            $keymaps->{$name} = $keymap;
          }
      }

    return undef if $errors;
    return undef if $warnings and $strict;
    return $self;
  }

sub print_nested
  {
    my $self = shift;
    my $fh = shift;
    my $nesting = shift;
    my $str = shift;
    print $fh "  " x $nesting;
    print $fh $str;
    print $fh "\n";
    return 1;
  }

sub format_modifier_value
  {
    my $self = shift;
    my $value = shift;

    return $value->{name} if exists $value->{name};

    my $v = $value->{value};

    # There should be precisely one bit set
    my @bits = grep {defined $_} map {($v & (1 << $_)) ? $_ : undef} 0..15;
    if (scalar @bits != 1)
      {
        die "Invalid modifier value $v (should be a power of two)";
      }
    return $bits[0];
  }

sub format_modifier
  {
    my $self = shift;
    my $modifier = shift;

    return join(' | ', map {$self->format_modifier_value($_)} @$modifier);
  }

sub quote_string
  {
    my $str = shift;
    $str =~ s/"/\\"/;
    return '"' . $str . '"';
  }

my %has_modifier_arg = map{$_=>1} qw/setModifiers maskModifiers toggleModifiers
                                     setStickyModifiers maskStickyModifiers toggleStickyModifiers/;
my %has_string_arg = map{$_=>1} qw/beginExtended unsetOption string event/;
my %has_list_arg = map{$_=>1} qw/setKeymap addKeymap removeKeymap/;
my %has_no_arg = map{$_=>1} qw/abortExtended clear/;

sub format_seq_member
  {
    my $self = shift;
    my $member = shift;

    if (exists $member->{keycode})
      {
        my $no_modifiers = scalar @{$member->{modifiers}} == 0;
        my $no_mask = scalar @{$member->{mask}} == 0;
        my $default_mask =
          (scalar @{$member->{mask}} == 1)
          && exists $member->{mask}[0]{value}
          && ($member->{mask}[0]{value} == (1 << 16) - 1);

        my $dir_str = $member->{direction} ? "" : "^";

        my $mod_str;
        if ($no_modifiers and $no_mask)
          {
            $mod_str = "[*]";
          }
        elsif ($no_modifiers and $default_mask)
          {
            $mod_str = "";
          }
        elsif ($default_mask)
          {
            $mod_str = "[" . $self->format_modifier($member->{modifiers}) . "]";
          }
        else
          {
            $mod_str = "[" . $self->format_modifier($member->{modifiers}) . "/" .  $self->format_modifier($member->{mask}) . "]";
          }

        return join('', $dir_str, $member->{keycode}, $mod_str);
      }
    else
      {
        if ($has_modifier_arg{$member->{action}})
          {
            return join('',
                        "$member->{action}(",
                        $member->{arg}{not} ? "!" : "",
                        $self->format_modifier($member->{arg}{modifiers}),
                        ")");
          }
        elsif ($has_string_arg{$member->{action}})
          {
            return "$member->{action}(" . quote_string($member->{arg}) . ")";
          }
        elsif ($has_list_arg{$member->{action}})
          {
            return "$member->{action}(" . join(', ', (map {quote_string($_)} @{$member->{arg}})) . ")";
          }
        elsif ($has_no_arg{$member->{action}})
          {
            return "$member->{action}()";
          }
        elsif ($member->{action} eq 'setOption')
          {
            return "setOption(" . join(', ', (map {quote_string($_)} $member->{arg}{option}, $member->{arg}{keymap})) . ")";
          }
        else
          {
            die "Unhandled action '$member->{action}'";
          }
      }
  }

sub format_seq
  {
    my $self = shift;
    my $seq = shift;

    my @members = map {$self->format_seq_member($_)} @$seq;
    for (my $i = 0; $i < $#members - 1; $i++)
      {
        # If the next member is precisely equal to this one, only with
        # a ^ prefix, then this is a down-up pair - so splice a *
        # prefix in place of the pair.
        if ('^' . $members[$i] eq $members[$i + 1])
          {
            splice @members, $i, 2, '*' . $members[$i];
          }
      }
    return join(' ', @members);
  }

sub output_keymap
  {
    my $self = shift;
    my $fh = shift;
    my $name = shift;
    my %keymap = %{shift()};
    my $nested = shift || 0;

    print $fh "\n" unless $nested;

    $self->print_nested($fh, $nested, "keymap" . ($name ? (" " . quote_string($name)) : ""));
    $self->print_nested($fh, $nested, "{");

    my $need_space = 0;

    if (scalar keys %{$keymap{keycode}} > 4)
      {
        $self->print_nested($fh, $nested + 1, "keycode");
        $self->print_nested($fh, $nested + 1, "{");
        foreach my $keycode (sort keys %{$keymap{keycode}})
          {
            if (exists $keymap{keycode}{$keycode}{value})
              {
                $self->print_nested($fh, $nested + 2, "$keycode = $keymap{keycode}{$keycode}{value};");
              }
            else
              {
                $self->print_nested($fh, $nested + 2, "$keycode = $keymap{keycode}{$keycode}{alias};");
              }
          }
        $self->print_nested($fh, $nested + 1, "}");
        $need_space = 1;
      }
    elsif (scalar keys %{$keymap{keycode}} > 0)
      {
        foreach my $keycode (sort keys %{$keymap{keycode}})
          {
            if (exists $keymap{keycode}{$keycode}{value})
              {
                $self->print_nested($fh, $nested + 1, "keycode $keycode = $keymap{keycode}{$keycode}{value};");
              }
            else
              {
                $self->print_nested($fh, $nested + 1, "keycode $keycode = $keymap{keycode}{$keycode}{alias};");
              }
          }
        $need_space = 1;
      }

    print $fh "\n" if $need_space;
    $need_space = 0;

    if (scalar keys %{$keymap{modifier}} > 4)
      {
        $self->print_nested($fh, $nested + 1, "modifier");
        $self->print_nested($fh, $nested + 1, "{");
        foreach my $modifier (sort keys %{$keymap{modifier}})
          {
            my $value = $self->format_modifier($keymap{modifier}{$modifier}{value});
            $self->print_nested($fh, $nested + 2, "$modifier = $value;");
          }
        $self->print_nested($fh, $nested + 1, "}");
        $need_space = 1;
      }
    elsif (scalar keys %{$keymap{modifier}} > 0)
      {
        foreach my $modifier (sort keys %{$keymap{modifier}})
          {
            my $value = $self->format_modifier($keymap{modifier}{$modifier}{value});
            $self->print_nested($fh, $nested + 1, "modifier $modifier = $value;");
          }
        $need_space = 1;
      }

    print $fh "\n" if $need_space;
    $need_space = 0;

    if (scalar keys %{$keymap{'keycode name'}} > 4)
      {
        $self->print_nested($fh, $nested + 1, "keycode name");
        $self->print_nested($fh, $nested + 1, "{");
        foreach my $keycode (keys %{$keymap{'keycode name'}})
          {
            my $name = $keymap{'keycode name'}{$keycode}{name};
            $self->print_nested($fh, $nested + 2, "$keycode = " . quote_string($name) . ";");
          }
        $self->print_nested($fh, $nested + 1, "}");
        $need_space = 1;
      }
    elsif (scalar keys %{$keymap{'keycode name'}} > 0)
      {
        foreach my $keycode (keys %{$keymap{'keycode name'}})
          {
            my $name = $keymap{'keycode name'}{$keycode}{name};
            $self->print_nested($fh, $nested + 1, "keycode name $keycode = " . quote_string($name) . ";");
          }
        $need_space = 1;
      }

    print $fh "\n" if $need_space;
    $need_space = 0;

    if (scalar @{$keymap{'modifier name'}} > 4)
      {
        $self->print_nested($fh, $nested + 1, "modifier name");
        $self->print_nested($fh, $nested + 1, "{");
        foreach my $modifier (@{$keymap{'modifier name'}})
          {
            my $mod_str = $self->format_modifier($modifier->{modifier});
            my $mask_str = $self->format_modifier($modifier->{mask});
            $self->print_nested($fh, $nested + 2, "$mod_str/$mask_str = " . quote_string($modifier->{name}) . ";");
          }
        $self->print_nested($fh, $nested + 1, "}");
        $need_space = 1;
      }
    elsif (scalar @{$keymap{'modifier name'}} > 0)
      {
        foreach my $modifier (@{$keymap{'modifier name'}})
          {
            my $mod_str = $self->format_modifier($modifier->{modifier});
            my $mask_str = $self->format_modifier($modifier->{mask});
            $self->print_nested($fh, $nested + 1, "modifier name $mod_str/$mask_str = " . quote_string($modifier->{name}) . ";");
          }
        $need_space = 1;
      }

    print $fh "\n" if $need_space;
    $need_space = 0;

    if (scalar @{$keymap{seq}} > 4)
      {
        $self->print_nested($fh, $nested + 1, "seq");
        $self->print_nested($fh, $nested + 1, "{");
        foreach my $seq (@{$keymap{seq}})
          {
            $self->print_nested($fh, $nested + 2, $self->format_seq($seq) . ";");
          }
        $self->print_nested($fh, $nested + 1, "}");
      }
    elsif (scalar @{$keymap{seq}} > 0)
      {
        foreach my $seq (@{$keymap{seq}})
          {
            $self->print_nested($fh, $nested + 1, "seq " . $self->format_seq($seq) . ";");
          }
      }

    $self->print_nested($fh, $nested, "}");
  }

sub output_option
  {
    my $self = shift;
    my $fh = shift;
    my $name = shift;
    my $option = shift;

    print $fh "\n";
    print $fh "option " . quote_string($name) . "\n";
    print $fh "{\n";
    foreach my $keymap (keys %{$option->{keymap}})
      {
        if (ref $option->{keymap}{$keymap} eq 'HASH')
          {
            $self->output_keymap($fh, $keymap, $option->{keymap}{$keymap}, 1);
          }
        else
          {
            $self->print_nested($fh, 1, "keymap " . quote_string($keymap) . ";");
          }
      }
    print $fh "}\n";
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

    print $fh "# Generated from a complied keymap by ykbcomp\n";

    $self->output_keymap($fh, $_, $self->{keymap}{$_}) foreach keys %{$self->{keymap}};
    $self->output_option($fh, $_, $self->{option}{$_}) foreach keys %{$self->{option}};

    return 1;
  }

sub ykm
  {
    my $self = shift;

    my $ykm = new YKB::YKM $self->{keymap}, $self->{option};
    return $ykm;
  }

1;
