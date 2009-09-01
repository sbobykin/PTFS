#!/usr/bin/ruby

require 'find'

@ptfs_path = ARGV[0]
@scaned = {}
in_ptfs = []
pkg_list = [ARGV[1]]

def cat(path)
	f = File.new(path)
	retval = f.read()
	f.close
	return retval
end

def scan(package)
	if @scaned[package]
		return []
	end

	pkg_list = []
	mod = {}
	filename = package.downcase + ".ml"
	root_path = @ptfs_path + "/" + filename
	Find.find(root_path) do |path|
		if FileTest.directory?(path)
			case File.basename(path)[4..-1] 
			when "package" then
				pkg_list << cat(path + "/text")
				Find.prune
			when "module" then
				key = cat(path + "/000_mname/text")
				val = cat(path + "/001_pname/text")
				mod[key] = val
			end
		else 
			Find.prune
		end
	end

	pkg_list = pkg_list.sort.uniq

	pkg_list.each_index do |index|
		key = pkg_list[index]
		if(mod[key])
			pkg_list[index] = mod[key]
		end
	end

	@scaned[package] = true
	return pkg_list
end

def get_new_pkg_list(pkg_list)
	new_pkg_list = []
	pkg_list.each do |pkg|
		new_pkg_list += scan(pkg)
	end	
	return new_pkg_list = new_pkg_list.sort.uniq
end

Dir.glob(ARGV[0] + "/*.ml").each do |fname|
	pname = File.basename(fname).split(".")[0]
	pname[0..0] = pname[0..0].upcase
	#in_ptfs = in_ptfs + [pname]
	in_ptfs << pname
end
in_ptfs.sort.uniq

while (new = get_new_pkg_list(pkg_list)) != []
	pkg_list = (pkg_list + new).sort.uniq
end

(pkg_list & in_ptfs).each { |s| puts s.downcase + ".ml" }
