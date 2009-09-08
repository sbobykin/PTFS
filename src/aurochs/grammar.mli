(* Syntax/grammar *)
type node_name =
| N_And (* 0 *)
| N_Appending (* 1 *)
| N_Ascribe (* 2 *)
| N_BOF (* 3 *)
| N_Build (* 4 *)
| N_Char (* 5 *)
| N_Chars (* 6 *)
| N_Constant (* 7 *)
| N_EOF (* 8 *)
| N_Epsilon (* 9 *)
| N_Grammar (* 10 *)
| N_Insensitive (* 11 *)
| N_JustTag (* 12 *)
| N_Modifier (* 13 *)
| N_Modifying (* 14 *)
| N_N (* 15 *)
| N_Not (* 16 *)
| N_NotChars (* 17 *)
| N_Option (* 18 *)
| N_Or (* 19 *)
| N_Outfixing (* 20 *)
| N_Plus (* 21 *)
| N_Position (* 22 *)
| N_Prefixing (* 23 *)
| N_Prepending (* 24 *)
| N_Production (* 25 *)
| N_Range (* 26 *)
| N_Root (* 27 *)
| N_S (* 28 *)
| N_Sigma (* 29 *)
| N_Star (* 30 *)
| N_String (* 31 *)
| N_Suffixing (* 32 *)
| N_Surrounding (* 33 *)
| N_Tight (* 34 *)
| N_TightPlus (* 35 *)
| N_TightStar (* 36 *)
| N_Tokenize (* 37 *)


type attribute_name =
| A_close (* 0 *)
| A_high (* 1 *)
| A_low (* 2 *)
| A_name (* 3 *)
| A_open (* 4 *)
| A_value (* 5 *)


type tree = (node_name, attribute_name) Peg.poly_tree

type positioned_tree = (node_name, attribute_name) Peg.poly_positioned_tree


val print_node_name : out_channel -> node_name -> unit

val print_attribute_name : out_channel -> attribute_name -> unit

val binary : Aurochs.binary
val program : (node_name, attribute_name) Aurochs.program Lazy.t
val parse : string -> tree
val parse_positioned : string -> positioned_tree
val print_tree : out_channel -> tree -> unit
