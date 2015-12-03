lines = 0
function CountLines(filename)
	io.input(filename)
	for line in io.lines() do
		lines = lines + 1
	end	
	io.input()	
	print('current lines = ' .. lines)
end