" Vim syntax file for the msl programming language

if exists("b:current_syntax")
  finish
endif

" Keywords
syn keyword mslKeyword function let set if elif else return for in try expect 
syn keyword mslBuiltin print str int float len typeof list put at append prepend link copy panic error assert assert_type random 
syn keyword mslBuiltin ansi_color ansi_reset ansi_set_cursor ansi_move_cursor ansi_clear_line ansi_clear_screen ansi_clear
syn keyword mslBuiltin sys_env_get process_args sys_exit sys_exec sys_now sys_is_tty sys_has_color 
syn keyword mslBuiltin time_epoch_ms time_epoch_sec time_iso8601 
syn keyword mslBuiltin str_split str_replace str_contains str_has_prefix str_has_suffix str_lower str_upper str_trim str_slice str_find str_index str_fmt
syn keyword mslBuildin fs_exists fs_mkdir fs_rm fs_ls fs_read fs_write fs_append
syn keyword mslBuiltin syn keyword mslBuildin math_abs math_floor math_ceil math_round math_sqrt math_pow math_sin math_cos math_tan math_log math_exp
syn keyword mslBuildin bit_shift_left bit_shift_right bit_or bit_and bit_xor
syn keyword mslBuildin list_slice list_remove list_contains range

" Literals and Constants
syn keyword mslConstant none __MAIN__ __FILE__

" Comments
syn match mslComment ";.*$"

" Strings
syn region mslString start='"' end='"' skip='\\"'

" Symbols (starting with #, including #true, #false, #error, etc.)
syn match mslSymbol "#[a-zA-Z0-9_]\+"

" Numbers (integers and floats)
syn match mslNumber "\<\d\+\(\.\d\+\)\?\>"

" Preprocessor directives
syn match mslPreProc "\$\(include\|define\|fset\|funset\|iffset\|ifnfset\|endif\)\>"

" Operators
syn keyword mslOperator and or not
syn match mslOperator "+\|*\|-\|/\|%\|==\|!=\|<\|>\|<=\|>="

" Highlight groups
hi def link mslKeyword Statement
hi def link mslBuiltin Function
hi def link mslConstant Constant
hi def link mslComment Comment
hi def link mslString String
hi def link mslSymbol Constant
hi def link mslNumber Number
hi def link mslPreProc PreProc
hi def link mslOperator Operator

let b:current_syntax = "msl"
