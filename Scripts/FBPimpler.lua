------------------------------------------------------------------------------
-- This source file is part of fastbird engine
-- For the latest info, see http://www.jungwan.net/
-- 
-- Copyright (c) 2013-2015 Jungwan Byun
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.
------------------------------------------------------------------------------

filepath = arg[1]
print('filepath = ' .. filepath)
function FindFileName(path)
	local dotStart = string.find(path, '.cpp')	
	for i=dotStart, 1, -1 do
		if path:sub(i, i) == '\\' then
			return string.sub(filepath, i+1, dotStart - 1)
		end
	end
end
className = FindFileName(filepath)
print('className = ' .. className)
-- //#!PimplStart
-- //#!PimplEnd
function CaptureFunctionName(line)
	return string.match(line, "([%w_][%w%d_]+)%(")
end
local WholeLines;

function DeleteComments(line)
	local lineCommentStart = string.find(line, '//')
	local blockCommentStart = string.find(line, '/*', 1, true)
	if lineCommentStart then
		if not blockCommentStart or lineCommentStart < blockCommentStart then
			line = string.sub(1, lineCommentStart-1)
			return line
		end
	end
	
	if blockCommentStart then
		local _, blockCommentEnd = string.find(line, '*/', 1, true)
		if blockCommentEnd then
			line = string.gsub(line, '/\\*.*\\*/', '')
			return line
		else
			local notFound = true
			while notFound do	
				local nextLine = WholeLines()
				if not nextLine then
					print("cannot find a pair of /*")
					return ''
				end
				line = line .. nextLine
				local _, blockCommentEnd = string.find(nextLine, '*/', 1, true)
				if blockCommentEnd then
					line = string.gsub(line, '/\\*.*\\*/', '')
					return line				
				end				
			end		
		end
	end	
	return line
end
function RemoveLineFeed(line)
	while not string.find(line, ';') do
		local nextLine = WholeLines()
		if not nextLine then break end
		line = line .. nextLine
	end
	return line
end

function RemoveDefaultArg(line)
	line = string.gsub(line, "%s*=%s*[%w%d_]+", '')
	return line
end

function ReplaceToMethod(line, functionName)
	local functionStart = functionName .. "%s*%("
	return string.gsub(line, functionStart, string.format("%s::%s(", className, functionName))
end

function CaptureParamList(line, functionName)
	local list = ''
	local params = string.match(line, '%b()')
	params = string.gsub(params, "%s*=%s*0", '')
	params = string.gsub(params, "%(", '')
	--print('params = '.. params)	
	
	local first = true
	for param in string.gmatch(params, "([%w_][%w%d_]*)[,%)%[]") do
		if first then
			list = list .. param
			first = false
		else
			list = list .. ', ' .. param
		end
	end
	return list
end

function NeedToReturn(line)
	for returnType in string.gmatch(line, "[%w_][%w%d_]*%**") do		
		if (returnType == 'void') then
			return false;
		end
		return true;
	end
end

if filepath then
	io.input(filepath)
	
	local lines ={}
	local started = false
	local successful = false
	WholeLines = io.lines()
	for line in WholeLines do
		local specialLine= false
		if string.find(line, '//#!PimplStart') then
			started = true			
			specialLine = true
		elseif string.find(line, '//#!PimplEnd') then
			started = false			
			specialLine = true
		end
		if started then
			line = DeleteComments(line);
			local commentStart = string.find(line, '//')
			if commentStart==nil or commentStart > 4   then
				local functionName = CaptureFunctionName(line)
				if functionName then
					line = RemoveLineFeed(line)
					line = RemoveDefaultArg(line)
					line = ReplaceToMethod(line, functionName)
					--print('Funtion Name = ' .. functionName);
					local paramList = CaptureParamList(line, functionName)
					--print('param list = ' .. paramList)				
					--print('')
					local needToReturn = NeedToReturn(line)
					local new
					if needToReturn then
						new = string.gsub(line, ";", string.format(" {\n\treturn mImpl->%s(%s);\n}\n\n", functionName, paramList))
					else
						new = string.gsub(line, ";", string.format(" {\n\tmImpl->%s(%s);\n}\n\n", functionName, paramList))
					end
					lines[#lines+1] = new;
					successful = true
				end
			else
				if not specialLine then
					lines[#lines+1] = line .. '\n';
				end
			end
		else
			if not specialLine then
				lines[#lines+1] = line .. '\n';
			end
		end
	end
	io.input()	
	if (successful) then
		io.output(filepath)
		for i=1, #lines do
			io.write(lines[i])
		end
	end
	io.output()
end
io.read()