# gmod-setshaders-module

A Garry's Mod binary module that restores the ability to modify material shaders at runtime using the stable `IMaterial::SetShaderAndParams()` Source SDK method.

## What is this?

GMod's `IMaterial:SetShader()` was disabled years ago due to crashes, forcing developers to use the limited `CreateMaterial()` workaround which can't modify existing world materials. This module exposes the working `SetShaderAndParams` C++ method, allowing you to change shaders on ANY material - including world brushes - without crashes.

## Features

- ✅ Modify existing materials (including world/brush materials)
- ✅ Preserve material parameters when changing shaders
- ✅ No memory duplication like `CreateMaterial()`
- ✅ Batch material operations with helper functions
- ✅ Stable - uses existing Source SDK functionality

## Installation

1. Download the compiled module from [Releases](../../releases)
2. Extract `gmcl_materialshaderex_win32.dll` (or appropriate OS version)
3. Place it in `Steam\steamapps\common\GarrysMod\garrysmod\lua\bin\`
4. Restart Garry's Mod

## Usage

### Require the module
```lua
require("materialshaderex")
```

### Available Functions

#### Material Methods

**`IMaterial:SetShaderAndParams(table keyvalues)`**

Changes shader and parameters with full control. You specify everything from scratch.

```lua
local mat = Material("concrete/concrete_wall")
mat:SetShaderAndParams({
    shader = "PBR",
    ["$basetexture"] = "concrete/concrete_wall",
    ["$bumpmap"] = "concrete/concrete_wall_normal",
    ["$mraotexture"] = "dev/pbr_mraotexture"
})
mat:Recompute()
```

**`IMaterial:SetShaderName(string shaderName)`**

Convenience wrapper that automatically preserves all existing material parameters when changing shader.

```lua
local mat = Material("models/props/chair")
mat:SetShaderName("PBR") -- Keeps all existing textures/params
mat:Recompute()
```

#### Global Helper Functions

**`MaterialShaderEx.GetAllMaterials()`**

Returns a table of all currently loaded material names.

```lua
local allMats = MaterialShaderEx.GetAllMaterials()
print("Total materials loaded:", #allMats)
```

**`MaterialShaderEx.GetMaterialsByGroup(string groupName)`**

Returns materials filtered by texture group.

```lua
-- Get all world/brush materials
local worldMats = MaterialShaderEx.GetMaterialsByGroup("World textures")

-- Other useful groups:
-- "Model textures", "SkyBox textures", "Particle textures", etc.
```

**`MaterialShaderEx.ChangeShader(string materialName, string shaderName)`**

Convenience function to change shader by material name (returns success boolean).

```lua
local success = MaterialShaderEx.ChangeShader("concrete/wall", "PBR")
```

### Example: Convert Surface to PBR on Look

```lua
require("materialshaderex")

hook.Add("KeyPress", "ConvertToPBR", function(ply, key)
    if key ~= IN_USE then return end
    
    local tr = ply:GetEyeTrace()
    if not tr.Hit or not tr.HitTexture then return end
    
    local mat = Material(tr.HitTexture)
    local shader = mat:GetShader()
    
    -- Only convert LightmappedGeneric or VertexLitGeneric
    if shader == "LightmappedGeneric" or shader == "VertexLitGeneric" then
        mat:SetShaderName("PBR")
        mat:SetTexture("$mraotexture", "dev/pbr_mraotexture3")
        mat:Recompute()
        
        print("Converted:", tr.HitTexture, "to PBR")
    end
end)
```

## Known Limitations

- **Lightmap seams**: Converting LightmappedGeneric brush materials at runtime may cause lighting seams between brushes. This is because lightmaps are baked by the BSP compiler. A map restart fixes this. May be shader-specific.
- **Memory limits**: Converting large numbers of materials (100+) at once may cause issues. Use incremental/batch processing for better stability.

## Compiling from Source

### Prerequisites
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community Edition works)
- [Premake5](https://premake.github.io/download)
- [garrysmod_common](https://github.com/danielga/garrysmod_common)

### Build Steps

1. **Clone this repository**
   ```bash
   git clone https://github.com/yourusername/gmod-setshaders-module.git
   cd gmod-setshaders-module
   ```

2. **Set up garrysmod_common**
   ```bash
   git clone https://github.com/danielga/garrysmod_common.git
   cd garrysmod_common
   git submodule update --init --recursive
   cd ..
   ```

3. **Generate project files**
   ```bash
   premake5 vs2022
   ```

4. **Build the project**
   - Open the generated `.sln` file in Visual Studio
   - Select `Release` configuration
   - Build the solution (F7)

5. **Output**
   - The compiled DLL will be in the `build/` directory
   - Copy to `garrysmod/lua/bin/`

### Detailed Guide
For more information on compiling GMod binary modules, see the [official wiki](https://wiki.facepunch.com/gmod/Creating_Binary_Modules:_Premake).

## Use Cases

- 🎨 Runtime shader switching for visual mods
- 🔧 Shader development and testing
- 🌍 Dynamic map material modifications
- 🎮 Graphics quality modes
- 🛠️ Material editor tools

## Technical Details

- Uses `IMaterial::SetShaderAndParams()` from Source SDK (stable, non-crashing method)
- Properly handles all material var types (float, int, string, vector, texture, matrix)
- Skips internal auto-generated flags to prevent conflicts
- Includes proper KeyValues memory management

## Contributing

Issues and pull requests welcome! This is an experimental module, so feedback from the community is valuable.

## License

[MIT License](LICENSE)

## Credits

Built using [garrysmod_common](https://github.com/danielga/garrysmod_common) by danielga.

---

**Note**: This module is not officially supported by Facepunch. Use at your own risk. A feature request has been submitted to add this functionality to GMod officially but was closed soo :(
