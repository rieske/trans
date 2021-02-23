# Scanner finite automaton configuration
# Syntax:
#   :start_state
#   @transition_state1                    characters_for_transition
#   @transition_state2                    characters_for_transition
#   ...
#   @transition_stateN                    characters_for_transition
#
#   :[%|"|/]state1              [lexeme_identifier (positive integer)]
#   @transition_state           [characters_for_transition]
#   [@transition_state          [characters_for_transition] ]
#
#   ...
#   :final_state
#
#   %list of keywords
#
# Keywords will be assigned an identifier starting from 1 in accending order as they are defined.
# The identifiers for the non keyword lexemes have to be higher than the number of keywords.
# If there is a clash in identifiers, the behaviour is undefined
#
# State name prefixes:
#   % triggers a keyword lookup for lexemes of that state
#   " string literal - spaces will be consumed by the same state and appended to the lexeme
#   / end of line comment. Anything till the end of line will be discarded
#
# A @ transition without any characters is treated as a wildcard transition - if no characters from other transitions match,
# any character will trigger a transition to that state

:start
@identifier     abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_
@int_constant   0123456789
@literal_start  '
@string_start   "
@delimeter      ,
@semicolon      ;
@opening_brace  {
@closing_brace  }
@opening_brack  [
@closing_brack  ]
@opening_paren  (
@closing_paren  )
@star           *
@modulo         %
@slash          /
@hyphen         -
@plus           +
@equals         =
@bin_and        &
@bin_xor        ^
@bin_or         |
@not            !
@less_than      <
@more_than      >
@start

:literal_start
@literal_esc    \
@literal_end    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789

:literal_esc
@literal_end    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789'\

:literal_end
@literal        '

:"string_start
@string_esc     \
@string_start   abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789*/%-+=&^|!<>;:{}()[],'
@string         "

:"string_esc
@string_start   abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789*/%-+=&^|!<>;:{}()[],'\

:operator
@operator       =-+&|<>
@fin

:%identifier    id
@identifier     abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789
@fin

:int_constant   int_const
@int_constant   0123456789
@float_constant .
@fin

:float_constant float_const
@float_constant 0123456789
@fin

:literal        char_const
@fin

:string         string
@fin

:delimeter      ,
@fin

:opening_paren  (
@fin

:closing_paren  )
@fin

:star           *
@star_eq        =
@fin

:star_eq        *=
@fin

:modulo         %
@mod_eq         =
@fin

:mod_eq         %=
@fin

:slash          /
@eol_comment    /
@comment        *
@slash_eq       =
@fin

:slash_eq       /=
@fin

:comment
@comment_star   *
@comment

:comment_star
@comment_end    /
@comment_star   *
@comment

:comment_end
@fin

:/eol_comment
@fin

:hyphen         -
@minus_eq       =
@decrement      -
@fin

:minus_eq       -=
@fin

:decrement      --
@fin

:plus           +
@plus_eq        =
@increment      +
@fin

:plus_eq        +=
@fin

:increment      ++
@fin

:equals         =
@eq_test        =
@fin

:bin_and        &
@and_eq         =
@logical_and    &
@fin

:and_eq         &=
@fin

:logical_and    &&
@fin

:bin_xor        ^
@xor_eq         =
@fin

:xor_eq         ^=
@fin

:bin_or         |
@or_eq          =
@logical_or     |
@fin

:or_eq          |=
@fin

:logical_or     ||
@fin

:not            !
@not_eq         =
@fin

:eq_test        ==
@fin

:not_eq         !=
@fin

:less_than      <
@lt_eq          =
@shift_left     <
@fin

:lt_eq          <=
@fin

:more_than      >
@mt_eq          =
@shift_right    >
@fin

:mt_eq          >=
@fin

:shift_left     <<
@sl_eq          =
@fin

:sl_eq          <<=
@fin

:shift_right    >>
@sr_eq          =
@fin

:sr_eq          >>=
@fin

:semicolon      ;
@fin

:opening_brace  {
@fin

:closing_brace  }
@fin

:opening_brack  [
@fin

:closing_brack  ]
@fin

:fin

%int char void float
%if else
%while for continue break return
%input output

