#pragma once

#include <vector>
#include <string>

struct Pair {
	std::string bone1;
	std::string bone2;

	Pair(std::string b1, std::string b2)
	{
		bone1 = b1;
		bone2 = b2;
	}
};


std::vector<Pair> create_AtoT_gerarchy() {
	std::vector<Pair> AtoT_gerarchy;

	//Head - Neck
	//AtoT_gerarchy.push_back(Pair("Head", "Neck"));
	//AtoT_gerarchy.push_back(Pair("Neck", "Spine3"));

	//Right arm
	AtoT_gerarchy.push_back(Pair("R_Hand", "R_Wrist"));
	AtoT_gerarchy.push_back(Pair("R_Wrist", "R_Elbow"));
	AtoT_gerarchy.push_back(Pair("R_Elbow", "R_Sholder"));
	AtoT_gerarchy.push_back(Pair("R_Sholder", "R_Collar"));
	//AtoT_gerarchy.push_back(Pair("R_Collar", "Spine3"));

	//Left arm
	AtoT_gerarchy.push_back(Pair("L_Hand", "L_Wrist"));
	AtoT_gerarchy.push_back(Pair("L_Wrist", "L_Elbow"));
	AtoT_gerarchy.push_back(Pair("L_Elbow", "L_Shoulder"));
	AtoT_gerarchy.push_back(Pair("L_Shoulder", "L_Collar"));
	//AtoT_gerarchy.push_back(Pair("L_Collar", "Spine3"));

	//Right Leg
	AtoT_gerarchy.push_back(Pair("Pelvis", "R_Hip"));
	AtoT_gerarchy.push_back(Pair("R_Hip", "R_Knee"));
	AtoT_gerarchy.push_back(Pair("R_Knee", "R_Ankle"));
	AtoT_gerarchy.push_back(Pair("R_Ankle", "R_Foot"));

	//Left Leg
	AtoT_gerarchy.push_back(Pair("Pelvis", "L_Hip"));
	AtoT_gerarchy.push_back(Pair("L_Hip", "L_Knee"));
	AtoT_gerarchy.push_back(Pair("L_Knee", "L_Ankle"));
	AtoT_gerarchy.push_back(Pair("L_Ankle", "L_Foot"));

	//Spine
	//AtoT_gerarchy.push_back(Pair("Spine3", "Spine2"));
	//AtoT_gerarchy.push_back(Pair("Spine2", "Spine1"));
	//AtoT_gerarchy.push_back(Pair("Spine1", "Pelvis"));

	return AtoT_gerarchy;
}
