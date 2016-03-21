import qbs
import qbs.FileInfo

Product {
	Depends { name:"cpp" }

	type: ["application", "hex", "size"]
    name: "nrf-avrasp"

    property string mcu: "-mmcu=atmega8"

    targetName: name + ".elf"

	cpp.defines: [
        "F_CPU=12000000"
	]

	cpp.warningLevel: "default" //"all" // or "none", "default"
	cpp.treatWarningsAsErrors: true
	cpp.positionIndependentCode: false

    cpp.commonCompilerFlags: [
        mcu,
        "-Wall",
    ]

	Properties {
    	condition: qbs.buildVariant === "debug"
		cpp.debugInformation: true
        cpp.optimization: "small" // "none" or "fast"
	}

	Properties {
		condition: qbs.buildVariant === "release"
		cpp.debugInformation: false
		cpp.optimization: "small"
	}

	cpp.linkerFlags:[
        mcu,
	]

	cpp.includePaths: [
	]

	files: [
        "./*.h",
        "./*.c",
        "./usbdrv/usbdrv.*",
        "./usbdrv/oddebug.*",
        "./usbdrv/usbportability.h",
        "./usbdrv/usbdrvasm.S",
        "../common/defines.h",
    ]

	Rule {
		id: hex
		inputs: "application"
		Artifact {
			fileTags: ["hex"]
			filePath: FileInfo.baseName(input.filePath) + ".hex"
		}
		prepare: {
			var args = ["-j", ".text", "-j", ".data", "-O", "ihex", input.filePath, output.filePath];
			var cmd = new Command("avr-objcopy", args);
			cmd.description = "converting to hex: " + FileInfo.fileName(input.filePath);
			cmd.highlight = "linker";
			return cmd;
		}
	}

	Rule {
        id: size
        inputs: ["hex"]
        Artifact {
            fileTags: ["size"]
            filePath: "-"
        }
        prepare: {
            var args = [input.filePath];
            var cmd = new Command("avr-size", args);
            cmd.description = "File size: " + FileInfo.fileName(input.filePath);
            cmd.highlight = "linker";
            return cmd;
        }
    }
}

