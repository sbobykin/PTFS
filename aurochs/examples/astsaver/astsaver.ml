(* Astsaver *)

open Aurochs_pack;;

let _ =
  if Array.length Sys.argv < 2 then
    begin
      Printf.eprintf "Usage: %s <grammar.peg> <file1> <file2> ...\n" Sys.argv.(0);
      Printf.eprintf "\n";
      Printf.eprintf "Will parse files file1, ... using the grammar and marshal the parse tree to file1.bin, ...\n";
      exit 1
    end;
  let peg = Sys.argv.(1) in
  for i = 2 to Array.length Sys.argv - 1 do
    begin
      let fn = Sys.argv.(i) in
      Printf.printf "%s\n%!" fn;
      try
        let t = Aurochs.read_positioned ~grammar:(`Binary(`File peg)) ~text:(`File fn) in
        let fn' = (Filename.chop_extension (Filename.basename fn))^".bin" in
        Util.with_binary_file_output fn' (fun oc -> Marshal.to_channel oc t []);
      with
      | Aurochs.Parse_error n -> Printf.printf "%s: parse error at %d\n%!" fn n
    end;
    Gc.compact ()
  done
;;
