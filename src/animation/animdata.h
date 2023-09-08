#pragma once

#include<glm.hpp>

struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id = 0;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset = glm::mat4(1.0f);
};
#pragma once
