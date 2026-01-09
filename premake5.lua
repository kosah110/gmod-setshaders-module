PROJECT_GENERATOR_VERSION = 2

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "./garrysmod_common"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon)

CreateWorkspace({
	name = "materialshaderex",
	abi_compatible = false,
	path = "projects/" .. os.target() .. "/" .. _ACTION
})

	-- Server-side module
	CreateProject({
		serverside = true,
		source_path = "source",
		manual_files = false
	})
		IncludeLuaShared()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeSDKTier2()
		IncludeSDKTier3()
		IncludeSDKMathlib()
		
		-- Add materialsystem includes
		includedirs({
			gmcommon .. "/sourcesdk-minimal/public",
			gmcommon .. "/sourcesdk-minimal/public/tier0",
			gmcommon .. "/sourcesdk-minimal/public/tier1",
			gmcommon .. "/sourcesdk-minimal/public/tier2",
			gmcommon .. "/sourcesdk-minimal/public/tier3",
			gmcommon .. "/sourcesdk-minimal/public/materialsystem"
		})
		
		-- Link against materialsystem
		filter("system:windows")
			links({
				"tier0",
				"tier1",
				"tier2",
				"tier3",
				"mathlib",
				"vstdlib"
			})
		
		filter("system:linux")
			links({
				"tier0_srv",
				"vstdlib_srv"
			})

	-- Client-side module (optional, if you need client-side functionality)
	CreateProject({
		serverside = false,
		source_path = "source",
		manual_files = false
	})
		IncludeLuaShared()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeSDKTier2()
		IncludeSDKTier3()
		IncludeSDKMathlib()
		
		-- Add materialsystem includes
		includedirs({
			gmcommon .. "/sourcesdk-minimal/public",
			gmcommon .. "/sourcesdk-minimal/public/tier0",
			gmcommon .. "/sourcesdk-minimal/public/tier1",
			gmcommon .. "/sourcesdk-minimal/public/tier2",
			gmcommon .. "/sourcesdk-minimal/public/tier3",
			gmcommon .. "/sourcesdk-minimal/public/materialsystem"
		})
		
		-- Link against materialsystem
		filter("system:windows")
			links({
				"tier0",
				"tier1",
				"tier2",
				"tier3",
				"mathlib",
				"vstdlib"
			})
		
		filter("system:linux")
			links({
				"tier0",
				"vstdlib"
			})