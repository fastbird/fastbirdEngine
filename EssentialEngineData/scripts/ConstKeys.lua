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

--keys.lua
ConstKeys=
{
	BACK=8,
	ENTER=13,
	SHIFT=16,
	CONTROL=17,
	ALT=18,
	ESC=27,
	SPACE=32,
	LEFT=37,
	UP=38,
	RIGHT=39,
	DOWN=40,
	["0"]=48,
	["1"]=49,
	["2"]=50,
	["3"]=51,
	["4"]=52,
	["5"]=53,
	["6"]=54,
	["7"]=55,
	["8"]=56,
	["9"]=57,
	A=65,
	B=66,
	C=67,
	D=68,
	E=69,
	F=70,
	G=71,
	H=72,
	I=73,
	J=74,
	K=75,
	L=76,
	M=77,
	N=78,
	O=79,
	P=80,
	Q=81,
	R=82,
	S=83,
	T=84,
	U=85,
	V=86,
	W=87,
	X=88,
	Y=89,
	Z=90,	
};

ConstKeysRev={};
for k, v in pairs(ConstKeys) do
	ConstKeysRev[v]=k;
end