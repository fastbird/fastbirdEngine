/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "FBColladaData.h"
#include "FBDebugLib/Logger.h"
#include "FBStringLib/StringLib.h"

namespace fb{
	namespace collada{
		ColShape ConvertColShapeStringToEnum(const char* colShape){
			if (strcmp(colShape, "sphere") == 0){
				return ColShapeSphere;
			}
			else if(strcmp(colShape, "cube") == 0){
				return ColShapeCube;
			}
			else if (strcmp(colShape, "mesh") == 0){
				return ColShapeMesh;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot convert colshape string(%s) to enum", colShape).c_str());
				return ColShapeSphere;
			}
		}
		Vec3 Vec3::operator - (const Vec3& r) const{
			return Vec3(x - r.x, y - r.y, z - r.z);
		}
		Vec3 Vec3::Cross(const Vec3& rVector) const{
			return Vec3(
				y * rVector.z - z * rVector.y,
				z * rVector.x - x * rVector.z,
				x * rVector.y - y * rVector.x);
		}
		float Vec3::Dot(const Vec3& vec) const{
			return x * vec.x + y * vec.y + z * vec.z;
		}

		float Vec3::Normalize(){
			float length = sqrt(x*x + y*y + z*z);
			if (length > 0.0f)
			{
				float invLength = 1.f / length;
				x *= invLength;
				y *= invLength;
				z *= invLength;
			}

			return length;
		}

		Vec3 Vec3::NormalizeCopy() const{
			Vec3 result = *this;
			result.Normalize();
			return result;
		}
	}
}