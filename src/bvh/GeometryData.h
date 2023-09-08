#pragma once
// -------------------------------- Ball --------------------------------
Vec3 ballPos(-0.580875, 11.230480, 1.876250);
float ballRadius = 0.05;
glm::vec4 ballColor(0.0f, 1.0f, 0.0f, 1.0f);
Ball ball(ballPos, ballRadius, ballColor);


// -------------------------------- Ground --------------------------------
Vec3 groundPos(-10, -8, 8);
//Vec3 groundPos(-14, -8, 8);
Vec2 groundSize(20, 15);
//Vec2 groundSize(30, 25);
glm::vec4 groundColor(0.8, 0.8, 0.8, 1.0);
//glm::vec4 groundColor(0.5, 1.0, 0.2, 1.0);
Ground ground(groundPos, groundSize, groundColor);




// -------------------------------- Path --------------------------------
string TshirtBig_path = "Mesh/Tshirt/TshirtTposeBig.obj";
string TshirtReduct_path = "Mesh/Tshirt/TshirtTposeReduct.obj";
string TshirtWoman_path = "Mesh/Woman/Tshirt/TshirtWoman3.obj";
string TankTop_path = "Mesh/Tanktop/TanktopResize2.obj";
string Tissue_path = "Mesh/Tshirt/Tissue2.obj";
string Skirt_path = "Mesh/Skirt/Skirt5.obj";
string Top_path = "Mesh/Top/Top3.obj";


// -------------------------------- Structure Tshirt --------------------------------
glm::vec3 translationTshirt = glm::vec3(0.0f, -1.5f, 0.7f);
glm::vec3 scaleTshirt = glm::vec3(0.044f, 0.034f, 0.042f);

// -------------------------------- Structure Tshirt woman --------------------------------
glm::vec3 translationTshirtWoman = glm::vec3(0.08f, -1.7f, -0.73f);
glm::vec3 scaleTshirtWoman = glm::vec3(6.0f, 5.0f, 6.0f);

// -------------------------------- Structure Cloth --------------------------------
glm::vec3 translationCloth = glm::vec3(-1.0f, 9.25f, 2.91f);
glm::vec3 scaleCloth = glm::vec3(0.044f, 0.034f, 0.042f);

// -------------------------------- Structure Tanktop --------------------------------
glm::vec3 translationTanktop = glm::vec3(0.0f, -3.7f, -0.42f);
glm::vec3 scaleTanktop = glm::vec3(0.5f, 0.39f, 0.7f);

// -------------------------------- Structure Tissue --------------------------------
glm::vec3 translationTissue = glm::vec3(-0.7f, 11.8f, 3.6f);
glm::vec3 scaleTissue = glm::vec3(4.0f, 3.0f, 0.0f);

// -------------------------------- Structure Skirt --------------------------------
glm::vec3 translationSkirt = glm::vec3(0.08f, -3.5f, -0.64f);
glm::vec3 scaleSkirt = glm::vec3(1.55f, 1.15f, 1.09f);

// -------------------------------- Structure Top --------------------------------
glm::vec3 translationTop = glm::vec3(0.09f, -3.4f, -0.7f);
glm::vec3 scaleTop = glm::vec3(6.76f, 6.9f, 6.2f);

// -------------------------------- Skirt constraint --------------------------------
const int skirt_dim = 16;
int skirt_pos[skirt_dim] = {96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111};
int skirt_human_pos[skirt_dim] = {916, 4402, 5245, 1779, 4425, 6375, 938, 705, 4406, 6377, 918, 2912, 2916, 4167, 6371, 2915};

// -------------------------------- Top constraint --------------------------------
const int top_dim = 34;
int top_pos[top_dim] = {         3,    4,    5,    6,    8,   30,   32,   49, 50,   51,  52,   66,   85,  86,   104,  127, 128,  129,  150,   151, 152,  159,  180,  181,  182,  183,  184,  198,  199,  200,  205,   207,  209,  222 };
int top_human_pos[top_dim] = { 1827, 1809, 2894, 1861, 1863, 1872, 1239, 733, 803, 520, 3011, 2894, 523, 1863, 5333, 5324, 5356, 4721, 6353, 4230, 5270, 6469, 5270, 6351, 5325, 5294, 5292, 6346, 5292, 5342, 1873, 1316, 1809, 5276 };




//Get Functions
glm::vec3 getTranslation() {
	if (meshType == TSHIRT_BIG || meshType == TSHIRT_REDUCT)
		return translationTshirt;
	if (meshType == CLOTH)
		return translationCloth;
	if (meshType == TANKTOP)
		return translationTanktop;
	if (meshType == TISSUE)
		return translationTissue;
	if (meshType == SKIRT)
		return translationSkirt;
	if (meshType == TOP)
		return translationTop;
	if (meshType == TSHIRT_WOMAN)
		return translationTshirtWoman;
}
glm::vec3 getScale() {
	if (meshType == TSHIRT_BIG || meshType == TSHIRT_REDUCT)
		return scaleTshirt;
	if (meshType == CLOTH)
		return scaleCloth;
	if (meshType == TANKTOP)
		return scaleTanktop;
	if (meshType == TISSUE)
		return scaleTissue;
	if (meshType == SKIRT)
		return scaleSkirt;
	if (meshType == TOP)
		return scaleTop;
	if (meshType == TSHIRT_WOMAN)
		return scaleTshirtWoman;
}
string getClothPath() {
	if (meshType == TSHIRT_BIG)
		return TshirtBig_path;
	if (meshType == TSHIRT_REDUCT)
		return TshirtReduct_path;
	if (meshType == TANKTOP)
		return TankTop_path;
	if (meshType == TISSUE)
		return Tissue_path;
	if (meshType == SKIRT)
		return Skirt_path;
	if (meshType == TOP)
		return Top_path;
	if (meshType == TSHIRT_WOMAN)
		return TshirtWoman_path;
}

vector<int> getSkirtConstrint() {
	vector<int> result;
	for (int i = 0; i < skirt_dim; i++)
		result.push_back(skirt_pos[i]);
	return result;
}

vector<int> getSkirtHumanConstraint() {
	vector<int> result;
	for (int i = 0; i < skirt_dim; i++)
		result.push_back(skirt_human_pos[i]);
	return result;
}

vector<int> getTopConstrint() {
	vector<int> result;
	for (int i = 0; i < top_dim; i++)
		result.push_back(top_pos[i]);
	return result;
}

vector<int> getTopHumanConstraint() {
	vector<int> result;
	for (int i = 0; i < top_dim; i++)
		result.push_back(top_human_pos[i]);
	return result;
}
