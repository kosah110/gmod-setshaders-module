#include "GarrysMod/Lua/Interface.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "tier1/KeyValues.h"
#include "tier1/interface.h"
#include "mathlib/vmatrix.h"

using namespace GarrysMod::Lua;

// Define the material system interface version
#ifndef MATERIAL_SYSTEM_INTERFACE_VERSION
#define MATERIAL_SYSTEM_INTERFACE_VERSION "VMaterialSystem080"
#endif

// Use the external material system pointer (defined in garrysmod_common)
extern IMaterialSystem* g_pMaterialSystem;

// Helper function to check if a parameter should be skipped
bool ShouldSkipParameter(const char* paramName)
{
	// Skip internal flags that are auto-generated
	if (strcmp(paramName, "$flags") == 0) return true;
	if (strcmp(paramName, "$flags_defined") == 0) return true;
	if (strcmp(paramName, "$flags2") == 0) return true;
	if (strcmp(paramName, "$flags_defined2") == 0) return true;
	if (strcmp(paramName, "$envmap") == 0) return true;

	return false;
}

// LUA: Material:SetShaderName(shaderName)
// Changes only the shader while preserving all existing parameters
LUA_FUNCTION(Material_SetShaderName)
{
	// Check if we have the material system
	if (!g_pMaterialSystem) {
		LUA->ThrowError("Material system not initialized!");
		return 0;
	}

	// Check arguments
	LUA->CheckType(1, Type::Material);
	LUA->CheckType(2, Type::String);

	// Get the material
	IMaterial* pMaterial = LUA->GetUserType<IMaterial>(1, Type::Material);
	if (!pMaterial) {
		LUA->ThrowError("Invalid material!");
		return 0;
	}

	// Get shader name
	const char* shaderName = LUA->GetString(2);

	// Get the current material's KeyValues to preserve parameters
	KeyValues* pOriginalKV = new KeyValues(shaderName);

	// Copy all shader parameters from the original material
	IMaterialVar** pParams;
	int paramCount = pMaterial->ShaderParamCount();
	pParams = pMaterial->GetShaderParams();

	// Iterate through all parameters and copy them
	for (int i = 0; i < paramCount; i++) {
		IMaterialVar* pVar = pParams[i];
		if (!pVar)
			continue;

		const char* paramName = pVar->GetName();

		// Skip internal flag parameters
		if (ShouldSkipParameter(paramName))
			continue;

		// Copy the parameter based on its type
		switch (pVar->GetType()) {
		case MATERIAL_VAR_TYPE_FLOAT:
			pOriginalKV->SetFloat(paramName, pVar->GetFloatValue());
			break;
		case MATERIAL_VAR_TYPE_INT:
			pOriginalKV->SetInt(paramName, pVar->GetIntValue());
			break;
		case MATERIAL_VAR_TYPE_STRING:
			pOriginalKV->SetString(paramName, pVar->GetStringValue());
			break;
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			const float* vec = pVar->GetVecValue();
			char vecStr[64];
			snprintf(vecStr, sizeof(vecStr), "[%f %f %f]", vec[0], vec[1], vec[2]);
			pOriginalKV->SetString(paramName, vecStr);
			break;
		}
		case MATERIAL_VAR_TYPE_TEXTURE:
		{
			ITexture* pTexture = pVar->GetTextureValue();
			if (pTexture) {
				pOriginalKV->SetString(paramName, pTexture->GetName());
			}
			break;
		}
		case MATERIAL_VAR_TYPE_MATERIAL:
		{
			IMaterial* pMat = pVar->GetMaterialValue();
			if (pMat) {
				pOriginalKV->SetString(paramName, pMat->GetName());
			}
			break;
		}
		case MATERIAL_VAR_TYPE_MATRIX:
		{
			const VMatrix& mat = pVar->GetMatrixValue();
			// Store matrix as string in row-major order
			char matStr[256];
			snprintf(matStr, sizeof(matStr), "[%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]",
				mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
				mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
				mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
				mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
			pOriginalKV->SetString(paramName, matStr);
			break;
		}
		case MATERIAL_VAR_TYPE_FOURCC:
			// FourCC is typically used for special formats, store as int
			pOriginalKV->SetInt(paramName, pVar->GetIntValue());
			break;
		case MATERIAL_VAR_TYPE_UNDEFINED:
			// Skip undefined types
			break;
		}
	}

	// Apply the new shader with preserved parameters
	pMaterial->SetShaderAndParams(pOriginalKV);

	// Refresh the material
	pMaterial->Refresh();

	// Clean up
	pOriginalKV->deleteThis();

	return 0;
}

// LUA: Material:SetShaderAndParams(keyvaluesTable)
// Changes the shader and parameters using a KeyValues table
LUA_FUNCTION(Material_SetShaderAndParams)
{
	// Check if we have the material system
	if (!g_pMaterialSystem) {
		LUA->ThrowError("Material system not initialized!");
		return 0;
	}

	// Check arguments
	LUA->CheckType(1, Type::Material);
	LUA->CheckType(2, Type::Table);

	// Get the material
	IMaterial* pMaterial = LUA->GetUserType<IMaterial>(1, Type::Material);
	if (!pMaterial) {
		LUA->ThrowError("Invalid material!");
		return 0;
	}

	// Create KeyValues from the Lua table
	KeyValues* pKeyValues = new KeyValues("shader");

	// Get shader name from table
	LUA->GetField(2, "shader");
	if (LUA->IsType(-1, Type::String)) {
		const char* shaderName = LUA->GetString(-1);
		pKeyValues->SetName(shaderName);
	}
	LUA->Pop();

	// Iterate through the table to get parameters
	LUA->PushNil();
	while (LUA->Next(2)) {
		// Get key and value
		LUA->Push(-2);
		const char* key = LUA->GetString(-1);

		// Skip the "shader" key since we handled it above
		if (strcmp(key, "shader") != 0) {
			// Check value type and set accordingly
			int valueType = LUA->GetType(-2);

			switch (valueType) {
			case Type::String:
				pKeyValues->SetString(key, LUA->GetString(-2));
				break;
			case Type::Number:
				pKeyValues->SetFloat(key, (float)LUA->GetNumber(-2));
				break;
			case Type::Bool:
				pKeyValues->SetInt(key, LUA->GetBool(-2) ? 1 : 0);
				break;
			}
		}

		LUA->Pop(2);
	}

	// Apply the shader and parameters
	pMaterial->SetShaderAndParams(pKeyValues);

	// Refresh the material
	pMaterial->Refresh();

	// Clean up
	pKeyValues->deleteThis();

	return 0;
}

// LUA: material.ChangeShader(materialName, shaderName)
// Convenience function to change shader by material name
LUA_FUNCTION(ChangeShader)
{
	if (!g_pMaterialSystem) {
		LUA->ThrowError("Material system not initialized!");
		return 0;
	}

	LUA->CheckType(1, Type::String);
	LUA->CheckType(2, Type::String);

	const char* materialName = LUA->GetString(1);
	const char* shaderName = LUA->GetString(2);

	// Find the material
	IMaterial* pMaterial = g_pMaterialSystem->FindMaterial(materialName, TEXTURE_GROUP_OTHER);

	if (!pMaterial || pMaterial->IsErrorMaterial()) {
		LUA->PushBool(false);
		return 1;
	}

	// Set the shader
	pMaterial->SetShader(shaderName);
	pMaterial->Refresh();

	LUA->PushBool(true);
	return 1;
}

// LUA: MaterialShaderEx.GetAllMaterials()
// Returns a table of all loaded materials
LUA_FUNCTION(GetAllMaterials)
{
	if (!g_pMaterialSystem) {
		LUA->ThrowError("Material system not initialized!");
		return 0;
	}

	// Create a table to hold all materials
	LUA->CreateTable();
	int tableIndex = 1;

	// Get material count
	int materialCount = g_pMaterialSystem->GetNumMaterials();

	// Iterate through all materials
	for (int i = 0; i < materialCount; i++) {
		IMaterial* pMaterial = g_pMaterialSystem->GetMaterial(i);

		if (!pMaterial || pMaterial->IsErrorMaterial()) {
			continue;
		}

		// Get material name
		const char* materialName = pMaterial->GetName();

		// Skip empty names
		if (!materialName || materialName[0] == '\0') {
			continue;
		}

		// Add to table
		LUA->PushNumber(tableIndex);
		LUA->PushString(materialName);
		LUA->SetTable(-3);

		tableIndex++;
	}

	return 1;
}

// LUA: MaterialShaderEx.GetMaterialsByGroup(groupName)
// Returns a table of materials filtered by texture group
LUA_FUNCTION(GetMaterialsByGroup)
{
	if (!g_pMaterialSystem) {
		LUA->ThrowError("Material system not initialized!");
		return 0;
	}

	LUA->CheckType(1, Type::String);
	const char* groupFilter = LUA->GetString(1);

	// Create a table to hold filtered materials
	LUA->CreateTable();
	int tableIndex = 1;

	// Get material count
	int materialCount = g_pMaterialSystem->GetNumMaterials();

	// Iterate through all materials
	for (int i = 0; i < materialCount; i++) {
		IMaterial* pMaterial = g_pMaterialSystem->GetMaterial(i);

		if (!pMaterial || pMaterial->IsErrorMaterial()) {
			continue;
		}

		// Get material name
		const char* materialName = pMaterial->GetName();

		// Skip empty names
		if (!materialName || materialName[0] == '\0') {
			continue;
		}

		// Check if texture group matches
		const char* texGroup = pMaterial->GetTextureGroupName();
		if (strcmp(texGroup, groupFilter) == 0) {
			// Add to table
			LUA->PushNumber(tableIndex);
			LUA->PushString(materialName);
			LUA->SetTable(-3);

			tableIndex++;
		}
	}

	return 1;
}

GMOD_MODULE_OPEN()
{
	// Initialize the material system interface if it's not already initialized
	if (!g_pMaterialSystem) {
		CreateInterfaceFn materialsystemFactory = Sys_GetFactory("materialsystem.dll");

		if (!materialsystemFactory) {
			LUA->ThrowError("Failed to get materialsystem.dll factory!");
			return 0;
		}

		g_pMaterialSystem = (IMaterialSystem*)materialsystemFactory(MATERIAL_SYSTEM_INTERFACE_VERSION, nullptr);

		if (!g_pMaterialSystem) {
			LUA->ThrowError("Failed to get IMaterialSystem interface!");
			return 0;
		}
	}

	// Add SetShader method to Material metatable
	// We need to get the metatable for the Material type
	LUA->PushSpecial(SPECIAL_REG); // Push registry
	LUA->GetField(-1, "IMaterial"); // Get IMaterial metatable

	if (LUA->IsType(-1, Type::Table)) {
		// Get the __index table
		LUA->GetField(-1, "__index");

		if (LUA->IsType(-1, Type::Table)) {
			// Add our functions to __index
			LUA->PushCFunction(Material_SetShaderName);
			LUA->SetField(-2, "SetShaderName");

			LUA->PushCFunction(Material_SetShaderAndParams);
			LUA->SetField(-2, "SetShaderAndParams");
		}

		LUA->Pop(); // Pop __index
	}

	LUA->Pop(); // Pop IMaterial metatable
	LUA->Pop(); // Pop registry

	// Create a material library table
	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->CreateTable();
	LUA->PushCFunction(ChangeShader);
	LUA->SetField(-2, "ChangeShader");
	LUA->PushCFunction(GetAllMaterials);
	LUA->SetField(-2, "GetAllMaterials");
	LUA->PushCFunction(GetMaterialsByGroup);
	LUA->SetField(-2, "GetMaterialsByGroup");
	LUA->SetField(-2, "MaterialShaderEx");
	LUA->Pop();
	return 0;
}

GMOD_MODULE_CLOSE()
{
	return 0;
}