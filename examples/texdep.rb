#!/usr/bin/ruby

require 'find'

root_path="/tmp/ptfs"
tex_root="../latex-test/"
cur_file = ""

def cat(path)
	f = File.new(path)
	retval = f.read()
	f.close
	return retval
end

ftop = {}
deps = {}

Find.find(tex_root) do |path|
	case File.basename(path)
	when /.*\.tex$/	
		ftop[File.basename(path)] =  path.sub(tex_root, "")
	end
end

Find.find(root_path) do |path|
	if FileTest.directory?(path)
		case File.basename(path)[4..-1] 
		when /.*\.tex$/
			cur_file =  File.basename(path)
		when "input" then
			loc_path = ftop[cat(path + "/text")] 
			if !deps[cur_file]
				deps[cur_file] = []
			end
			deps[cur_file] <<  ftop[cat(path + "/text") + ".tex"]
			Find.prune
		end
	else 
		Find.prune
	end
end

deps.each_key do |target|
	puts target + ": " + deps[target].join(" ")
end
