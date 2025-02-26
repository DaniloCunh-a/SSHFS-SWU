#@DESCRIPTION: Common Machine configuration for STM32 systems

require conf/machine/include/dn-lse-st-machine-extlinux-config-stm32mp.inc
require conf/machine/include/st-machine-features-stm32mp.inc
require conf/machine/include/dn-lse-st-machine-flashlayout-stm32mp.inc
require conf/machine/include/st-machine-flashlayout-deleteall-stm32mp.inc
require conf/machine/include/st-machine-flashlayout-extensible-stm32mp.inc
require conf/machine/include/st-machine-storage-device-stm32mp.inc

# Define specific common machine name
MACHINEOVERRIDES .= ":stcommon"

# Define specific common layer name
MACHINEOVERRIDES .= ":stm32mpcommon"

# =========================================================================
# SOC
# =========================================================================
STM32MP_SOC_NAME = ""
STM32MP_SOC_NAME:append:stm32mp15common = " stm32mp15 "
STM32MP_SOC_NAME:append:stm32mp13common = " stm32mp13 "
# Take care of nativesdk override use case
STM32MP_SOC_NAME:append:class-nativesdk = " stm32mp15 "
STM32MP_SOC_NAME:append:class-nativesdk = " stm32mp13 "

# =========================================================================
# boot scheme
# =========================================================================
# List of supported boot schemes
BOOTSCHEME_LABELS ??= "trusted optee"

# =========================================================================
# boot device
# =========================================================================
# List of supported boot devices
BOOTDEVICE_LABELS ??= "emmc nand-4-256 nor-sdcard sdcard"

# =========================================================================
# Machine settings
# =========================================================================
# Default machine feature
MACHINE_FEATURES = "usbhost usbgadget alsa screen ext2 pci"
MACHINE_FEATURES:append = " ${@bb.utils.contains('BOOTSCHEME_LABELS', 'optee', 'optee', '', d)} "
MACHINE_FEATURES:append = " tpm2 "

# Enable installation of INITRD image on bootfs side
MACHINE_FEATURES:append = " initrd "
# Remove InitRD package install from DISTRO_EXTRA_RRECOMMENDS to add it explicitly
# in our bootfs image (this avoid dependency loops as rootfs depends on bootfs...)
DISTRO_EXTRA_RRECOMMENDS:remove = " ${@bb.utils.contains('COMBINED_FEATURES', 'initrd', '${INITRD_PACKAGE}', '', d)} "

# Configure autoresize for any of the 'ext4' storage devices (through InitRD image)
MACHINE_FEATURES:append = " ${@bb.utils.contains_any('BOOTDEVICE_LABELS',  ['emmc', 'sdcard'], 'autoresize', '', d)} "

# Use FIP image for boot loaders
MACHINE_FEATURES:append = " fip"

# Enable firmware secure update feature
MACHINE_FEATURES:append = " fw-update"

# Default serial consoles (TTYs) to enable using getty
# Before kernel 4.18, serial console are ttyS3 but after is ttySTM0
SERIAL_CONSOLE = "115200 ttySTM0"

# Don't include kernels in standard images
RDEPENDS:${KERNEL_PACKAGE_NAME}-base = ""

# Ship all kernel modules by default
MACHINE_EXTRA_RRECOMMENDS = " kernel-modules kernel-imagebootfs"

# Default device tree list supported per board
STM32MP_DT_FILES_DK ??= ""
STM32MP_DT_FILES_ED ??= ""
STM32MP_DT_FILES_EV ??= ""
STM32MP_DT_FILES_DN ??= ""

# Set default supported device tree list
STM32MP_DEVICETREE:append = " ${STM32MP_DT_FILES_DK} "
STM32MP_DEVICETREE:append = " ${STM32MP_DT_FILES_ED} "
STM32MP_DEVICETREE:append = " ${STM32MP_DT_FILES_EV} "
STM32MP_DEVICETREE:append = " ${STM32MP_DT_FILES_DN} "

# =========================================================================
# Machine specific packages
# =========================================================================
# Enable Software watchdog when sysvinit
MACHINE_EXTRA_RRECOMMENDS:append = " ${@bb.utils.contains('DISTRO_FEATURES','sysvinit',' watchdog ','',d)} "

# =========================================================================
# Bootloader raw partition configuration : data, label, size (Kbytes)
# =========================================================================
# -----------------------------------------------------------------------------
# NOTE: There are few restrictions to follow:
#       -   The partition for the first boot loader should follow the naming
#           rule: fsbl*
#       -   The partition for the secondary boot loader should follow the naming
#           rule: fip
# -----------------------------------------------------------------------------
ENABLE_FLASHLAYOUT_CONFIG_FWUP ?= "${@bb.utils.contains('MACHINE_FEATURES', 'fw-update', '1', '0', d)}"
# Keep the binary copy configuration for legacy flashlayout file style
ENABLE_FLASHLAYOUT_PARTITION_BINCOPY = "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '0', '1', d)}"

STM32MP_FSBL_PROGAMMER_NAME ?= "fsbl-boot"
STM32MP_SSBL_PROGAMMER_NAME ?= "fip-boot"

STM32MP_FSBL1_DATA ?= "arm-trusted-firmware/tf-a-<TYPE>-<DEVICE>.stm32"
STM32MP_FSBL1_NAME ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', 'fsbl', 'fsbl1', d)}"
STM32MP_FSBL1_SIZE ?= "256"
STM32MP_FSBL2_DATA ?= "${STM32MP_FSBL1_DATA}"
STM32MP_FSBL2_NAME ?= "fsbl2"
STM32MP_FSBL2_SIZE ?= "${STM32MP_FSBL1_SIZE}"
STM32MP_SSBL1_DATA ?= "fip/fip-<TYPE>-<BOOTSCHEME>${FIP_SIGN_SUFFIX}.bin"
STM32MP_SSBL1_NAME ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', 'fip-a', 'fip', d)}"
STM32MP_SSBL1_SIZE ?= "4096"
STM32MP_SSBL2_DATA ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '', '${STM32MP_SSBL1_DATA}', d)}"
STM32MP_SSBL2_NAME ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', 'fip-b', 'fip2', d)}"
STM32MP_SSBL2_SIZE ?= "${STM32MP_SSBL1_SIZE}"
STM32MP_UENV_DATA ?= ""
STM32MP_UENV_NAME ?= "u-boot-env"
STM32MP_UENV_SIZE ?= "512"
STM32MP_METADATA_DATA ?= "${@bb.utils.contains('MACHINE_FEATURES', 'fw-update', 'arm-trusted-firmware/${TF_A_METADATA_BINARY}', '', d)}"
STM32MP_METADATA_NAME ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', 'metadata', '', d)}"
STM32MP_METADATA_SIZE ?= "256"

# Specific override for NAND device type regarding partition sizes to follow
# the hard coded configuration on U-Boot source code
STM32MP_FSBL1_SIZE_UBOOT ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '512', '1024', d)}"
STM32MP_METADATA_SIZE_UBOOT ?= "512"

# Bootloader Partitions configuration
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'emmc', 'emmc', '', d)}"
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'nand-4-256', '', d)}"
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor-sdcard', 'nor-sdcard', '', d)}"
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor', 'nor', '', d)}"
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'sdcard', 'sdcard', '', d)}"
PARTITIONS_BOOTLOADER_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'spinand-2-128', '', d)}"

# <binary_name>,<partlabel>,<size>,<type>,<copy>
PARTITIONS_BOOTLOADER_CONFIG[emmc] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE},Binary,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_FSBL2_DATA},${STM32MP_FSBL2_NAME},${STM32MP_FSBL2_SIZE},Binary,1' if '${STM32MP_FSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', False, True, d) else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,1' if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL2_SIZE},FIP,1' if '${STM32MP_SSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', True, False, d) else ''} \
    ${@ '${STM32MP_UENV_DATA},${STM32MP_UENV_NAME},${STM32MP_UENV_SIZE},Binary,1' if '${STM32MP_UENV_NAME}' else ''} \
    "
PARTITIONS_BOOTLOADER_CONFIG[nand-4-256] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE_UBOOT},Binary,2' if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE_UBOOT},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL1_SIZE},FIP,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_SSBL2_NAME}' else ''} \
    "
PARTITIONS_BOOTLOADER_CONFIG[nor] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE},Binary,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_FSBL2_DATA},${STM32MP_FSBL2_NAME},${STM32MP_FSBL2_SIZE},Binary,1' if '${STM32MP_FSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', False, True, d) else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,1' if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL2_SIZE},FIP,1' if '${STM32MP_SSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', True, False, d) else ''} \
    ${@ '${STM32MP_UENV_DATA},${STM32MP_UENV_NAME},${STM32MP_UENV_SIZE},Binary,1' if '${STM32MP_UENV_NAME}' else ''} \
    "
PARTITIONS_BOOTLOADER_CONFIG[nor-sdcard] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE},Binary,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_FSBL2_DATA},${STM32MP_FSBL2_NAME},${STM32MP_FSBL2_SIZE},Binary,1' if '${STM32MP_FSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', False, True, d) else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,1' if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL2_SIZE},FIP,1' if '${STM32MP_SSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', True, False, d) else ''} \
    ${@ '${STM32MP_UENV_DATA},${STM32MP_UENV_NAME},${STM32MP_UENV_SIZE},Binary,1' if '${STM32MP_UENV_NAME}' else ''} \
    "
PARTITIONS_BOOTLOADER_CONFIG[sdcard] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE},Binary,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_FSBL2_DATA},${STM32MP_FSBL2_NAME},${STM32MP_FSBL2_SIZE},Binary,1' if '${STM32MP_FSBL2_NAME}' and bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', False, True, d) else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,1' if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL2_SIZE},FIP,1' if '${STM32MP_SSBL2_NAME}' else ''} \
    ${@ '${STM32MP_UENV_DATA},${STM32MP_UENV_NAME},${STM32MP_UENV_SIZE},Binary,1' if '${STM32MP_UENV_NAME}' else ''} \
    "
PARTITIONS_BOOTLOADER_CONFIG[spinand-2-128] ?= "\
    ${@ '${STM32MP_FSBL1_DATA},${STM32MP_FSBL1_NAME},${STM32MP_FSBL1_SIZE_UBOOT},Binary,2' if '${STM32MP_FSBL1_NAME}' else ''} \
    ${@ '${STM32MP_METADATA_DATA},${STM32MP_METADATA_NAME},${STM32MP_METADATA_SIZE_UBOOT},Binary,2' if '${STM32MP_METADATA_NAME}' else ''} \
    ${@ '${STM32MP_SSBL1_DATA},${STM32MP_SSBL1_NAME},${STM32MP_SSBL1_SIZE},FIP,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_SSBL1_NAME}' else ''} \
    ${@ '${STM32MP_SSBL2_DATA},${STM32MP_SSBL2_NAME},${STM32MP_SSBL1_SIZE},FIP,' + bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '2', '1', d) if '${STM32MP_SSBL2_NAME}' else ''} \
    "

# =========================================================================
# Image
# =========================================================================
# For sparse, align on 4096b
IMAGE_ROOTFS_ALIGNMENT ?= "4"

# Enable licence summary and configure License content generation
ENABLE_IMAGE_LICENSE_SUMMARY ?= "1"

# Define image partition size (supposed to be set as max size in image recipe)

# Proposed value for rootfs should fit our highest constraint: NAND size (1GiB)
#   With fw-update we have the following partitions:
#       FSBL1 + FSBL2 + METADATA1+ METADATA2 + FIP-A1 + FIP-A2 + FIP-B1 + FIP-B2 + Multivolume UBI = NAND size
#       Multivolume UBI = 1GiB - (512KiB + 512KiB + 512KiB + 512KiB + 4MiB + 4MiB + 4MiB + 4MiB) = 1006MiB
#   Without fw-update we have following partitions
#       FSBL1 + FIP + FIP2 + Multivolume UBI = NAND size
#       Multivolume UBI = 1GiB - (2MiB + 4MiB + 4MiB) = 1014MiB
# With multivolume UBI split:
#   Multivolume UBI > uboot_config + uboot_config_r + bootfs + vendorfs + rootfs + userfs + UBI Overhead
# From http://www.linux-mtd.infradead.org/doc/ubi.html#L_overhead, we compute
# the UBI overhead for our NAND:
#   With fw-update:
#       (20*4096/1024 + 4) * 256KiB + (256KiB - 248KiB) * (1006MiB/256KiB - 20*4096/1024 - 4) = 53024KiB
#   Without fw-update:
#       (20*4096/1024 + 4) * 256KiB + (256KiB - 248KiB) * (1014MiB/256KiB - 20*4096/1024 - 4) = 53280KiB
# In addition, for each UBIFS, our NAND consummed 9 extra eraseblocks
# So:
#   rootfs < Multivolume UBI - (uboot_config + uboot_config_r + bootfs + vendorfs + userfs + UBI Overhead + 4 * 9*eraseblocks)
#   With fw-update:
#       rootfs < 1006MiB - (256KiB + 256KiB + 64MiB + 16MiB + 128MiB + 53024KiB + 4 * 9 * 256KiB)
#       rootfs < 736.7MiB
#   Without fw-update:
#       rootfs < 1014MiB - (256KiB + 256KiB + 64MiB + 16MiB + 128MiB + 53280KiB + 4 * 9 * 256KiB)
#       rootfs < 744.5MiB
# Proposed value for rootfs is:
#   With fw-update   : 736MiB
#   Without fw-update: 744MiB
STM32MP_ROOTFS_MAXSIZE_NAND ?= "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '753664', '762336', d)}"

# Default ROOTFS max size for image being built to this value
#IMAGE_ROOTFS_MAXSIZE ?= "${STM32MP_ROOTFS_MAXSIZE_NAND}"
IMAGE_ROOTFS_MAXSIZE ?= "260480"
#IMAGE_ROOTFSALT_MAXSIZE ?= "3145728"
IMAGE_ROOTFSALT_MAXSIZE ?= "1142008"

# Partitions configuration
IMAGE_CLASSES += "st-partitions-image"

# Enable use of extra partition(s)
ST_BOOTFS   ?= "1"
ST_VENDORFS ?= "1"
ST_USERFS   ?= "1"

PARTITIONS_IMAGES += "${@bb.utils.contains('ST_BOOTFS', '1', 'bootfs', '', d)}"
PARTITIONS_IMAGES += "${@bb.utils.contains('ST_VENDORFS', '1', 'vendorfs', '', d)}"
PARTITIONS_IMAGES += "rootfs"
PARTITIONS_IMAGES += "rootfsalt"
PARTITIONS_IMAGES += "${@bb.utils.contains('ST_USERFS', '1', 'userfs', '', d)}"

# Define image to use for extra partitions
STM32MP_BOOTFS_IMAGE      ?= "dn-lse-st-image-bootfs"
STM32MP_BOOTFS_LABEL      ?= "bootfs"
STM32MP_BOOTFS_MOUNTPOINT ?= "/boot"
# Proposed value for bootfs is 64MB
STM32MP_BOOTFS_SIZE       ?= "65536"


#ROOTFS BASE
STM32MP_ROOTFS_IMAGE ?= "diebold-image-base"
STM32MP_ROOTFS_LABEL ?= "rootfs"
# Configure the rootfs size with IMAGE_ROOTFS_MAXSIZE variable
STM32MP_ROOTFS_SIZE  ?= "${IMAGE_ROOTFS_MAXSIZE}"


#ROOTFS ALT
STM32MP_ROOTFSALT_IMAGE ?= "${IMAGE_BASENAME}"
STM32MP_ROOTFSALT_LABEL ?= "rootfsalt"
# Configure the rootfs size with IMAGE_ROOTFS_MAXSIZE variable
STM32MP_ROOTFSALT_SIZE  ?= "${IMAGE_ROOTFSALT_MAXSIZE}"


STM32MP_USERFS_IMAGE      ?= "st-image-userfs"
STM32MP_USERFS_LABEL      ?= "userfs"
STM32MP_USERFS_MOUNTPOINT ?= "/usr/local"
# Proposed value for userfs is 128MB
STM32MP_USERFS_SIZE       ?= "131072"


STM32MP_VENDORFS_IMAGE      ?= "st-image-vendorfs"
STM32MP_VENDORFS_LABEL      ?= "vendorfs"
STM32MP_VENDORFS_MOUNTPOINT ?= "/vendor"
# Proposed value for vendorfs is 16MB
STM32MP_VENDORFS_SIZE       ?= "16384"

# <image_name>,<partition_label>,<mountpoint>,<size>,<type>
PARTITIONS_IMAGES[bootfs]    ?= "${STM32MP_BOOTFS_IMAGE},${STM32MP_BOOTFS_LABEL},${STM32MP_BOOTFS_MOUNTPOINT},${STM32MP_BOOTFS_SIZE},System"
PARTITIONS_IMAGES[vendorfs]  ?= "${STM32MP_VENDORFS_IMAGE},${STM32MP_VENDORFS_LABEL},${STM32MP_VENDORFS_MOUNTPOINT},${STM32MP_VENDORFS_SIZE},FileSystem"
PARTITIONS_IMAGES[rootfs]    ?= "${STM32MP_ROOTFS_IMAGE},${STM32MP_ROOTFS_LABEL},,${STM32MP_ROOTFS_SIZE},FileSystem"
PARTITIONS_IMAGES[rootfsalt] ?= "${STM32MP_ROOTFS_IMAGE},${STM32MP_ROOTFSALT_LABEL},,${STM32MP_ROOTFSALT_SIZE},FileSystem"
PARTITIONS_IMAGES[userfs]    ?= "${STM32MP_USERFS_IMAGE},${STM32MP_USERFS_LABEL},${STM32MP_USERFS_MOUNTPOINT},${STM32MP_USERFS_SIZE},FileSystem"

# =========================================================================
# Image partition configuration : data, label, size (Kbytes)
# =========================================================================
STM32MP_UBIFS_DATA ?= "${IMAGE_LINK_NAME}_<CONFIG>_multivolume.ubi"
STM32MP_UBIFS_NAME ?= "ubifs"
STM32MP_UBIFS_SIZE ?= ""

# Partitions configuration
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'emmc', 'emmc', '', d)}"
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'nand-4-256', '', d)}"
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor', 'nor', '', d)}"
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor-sdcard', 'nor-sdcard', '', d)}"
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'sdcard', 'sdcard', '', d)}"
PARTITIONS_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'spinand-2-128', '', d)}"

PARTITIONS_SUFFIX ?= "ext4"

# <binary_name>,<partlabel>,<size>,<type>,<copy>
PARTITIONS_CONFIG[emmc] ?= "\
    ${@' '.join(['%s' % partImage2partConfig(config, '${PARTITIONS_SUFFIX}', d) for config in d.getVar('PARTITIONS_IMAGES').split()])} \
    "
PARTITIONS_CONFIG[nand-4-256] ?= "\
    ${STM32MP_UBIFS_DATA},${STM32MP_UBIFS_NAME},${STM32MP_UBIFS_SIZE},System,1 \
    "
PARTITIONS_CONFIG[nor] ?= "\
    ${@' '.join(['%s' % partImage2partConfig(config, '${PARTITIONS_SUFFIX}', d) for config in d.getVar('PARTITIONS_IMAGES').split()])} \
    "
PARTITIONS_CONFIG[nor-sdcard] ?= "\
    ${@' '.join(['%s' % partImage2partConfig(config, '${PARTITIONS_SUFFIX}', d) for config in d.getVar('PARTITIONS_IMAGES').split()])} \
    "
PARTITIONS_CONFIG[sdcard] ?= "\
    ${@' '.join(['%s' % partImage2partConfig(config, '${PARTITIONS_SUFFIX}', d) for config in d.getVar('PARTITIONS_IMAGES').split()])} \
    "
PARTITIONS_CONFIG[spinand-2-128]   ?= "\
    ${STM32MP_UBIFS_DATA},${STM32MP_UBIFS_NAME},${STM32MP_UBIFS_SIZE},System,1 \
"

# UBI Configuration
IMAGE_CLASSES += "image_types-stubi"

# Define UBI volume label to use in kernel command line to mount UBI file system
UBI_VOLNAME ?= "rootfs"

# Define UBI labels to build
#   Naming rules for UBI partitions:
#      nand_<PageSize>_<BlockSize>
#      nor_<BlockSize>
#   Like that a same UBI partition can be used for severals NAND/NOR providers
MULTIUBI_BUILD += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'nand_4_256', '', d)}"
MULTIUBI_BUILD += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'spinand_2_128', '', d)}"

# UBI Args for NAND by default on MB1262 (Micron MT29F8G16ABACAH4)
# LEB = BLOCK_SIZE - (2 * page size): 256*1024 - (2*4096)
MKUBIFS_ARGS:nand_4_256 = "--min-io-size 4096 --leb-size 253952 --max-leb-cnt 4096"
UBINIZE_ARGS:nand_4_256 = "--min-io-size 4096 --peb-size 256KiB"
# Set extra size required for UBIFS volume size (KiB)
EXTRA_UBIFS_SIZE:nand_4_256 = "2304"
# UBI Args for SPI NAND by default on MB1262 (Micron MT29F2G01ABA)
# LEB = BLOCK_SIZE - (2 * page size): 128*1024 - (2*2048)
MKUBIFS_ARGS:spinand_2_128 = "--min-io-size 2048 --leb-size 126976 --max-leb-cnt 2048"
UBINIZE_ARGS:spinand_2_128 = "--min-io-size 2048 --peb-size 128KiB"
# Set extra size required for UBIFS volume size (KiB)
EXTRA_UBIFS_SIZE:spinand_2_128 = "1408"

# Define two empty volumes to manage U-Boot config beginning of multivolume UBIFS
STM32MP_MULTIUBI_UENV1_LABEL = "uboot_config"
STM32MP_MULTIUBI_UENV1_SIZE ?= "256"
STM32MP_MULTIUBI_UENV2_LABEL = "uboot_config_r"
STM32MP_MULTIUBI_UENV2_SIZE ?= "${STM32MP_MULTIUBI_UENV1_SIZE}"

STM32MP_UBI_VOLUME:nand_4_256:prepend = "\
    ${STM32MP_MULTIUBI_UENV1_LABEL}:${STM32MP_MULTIUBI_UENV1_SIZE}:empty \
    ${STM32MP_MULTIUBI_UENV2_LABEL}:${STM32MP_MULTIUBI_UENV2_SIZE}:empty \
    "
STM32MP_UBI_VOLUME:spinand_2_128:prepend = "\
    ${STM32MP_MULTIUBI_UENV1_LABEL}:${STM32MP_MULTIUBI_UENV1_SIZE}:empty \
    ${STM32MP_MULTIUBI_UENV2_LABEL}:${STM32MP_MULTIUBI_UENV2_SIZE}:empty \
    "

# Manage specific override for UBI bootfs label name to 'boot' (hardcoded on U-Boot)
UBI_VOLNAME:pn-${STM32MP_BOOTFS_IMAGE} = "boot"

# Default FSTYPES requested
WKS_IMAGE_FSTYPES ?= ""
IMAGE_FSTYPES ?= "${WKS_IMAGE_FSTYPES} tar.xz"
# Append ext4 FSTYPES to default ones for emmc and sdcard volumes
IMAGE_FSTYPES += "${@bb.utils.contains_any('BOOTDEVICE_LABELS', 'emmc nor-sdcard sdcard', 'ext4', '', d)}"
# Append ubi FSTYPES to default ones for nand volumes
IMAGE_FSTYPES += "${@bb.utils.contains_any('BOOTDEVICE_LABELS', 'nand-4-256 spinand-2-128', 'stmultiubi', '', d)}"

# Define specific EXT4 command line:
#   - Create minimal inode number (as it is done by default in image_types.bbclass)
#   - Add label name (maximum length of the volume label is 16 bytes)
#     So use IMAGE_NAME_SUFFIX name by removing the '.' and truncing to 16 caracters
#   - Deactivate metadata_csum and dir_index (hashed b-trees): update not supported
#     by U-Boot
EXTRA_IMAGECMD:ext4 = "-i 4096 -L ${@d.getVar('IMAGE_NAME_SUFFIX').replace('.', '', 1)[:16]} -O ^metadata_csum,^dir_index"

# Allow debug on the platform with gdb and openocd tools
EXTRA_IMAGEDEPENDS:append = " \
    gdb-cross-${TARGET_ARCH} \
    openocd-stm32mp-native \
    stm32wrapper4dbg-native \
    sdcard-raw-tools-native \
    "

# Make sure to provide all expected tools in SDK
ST_TOOLS_FOR_SDK = " \
    nativesdk-gcc-arm-none-eabi \
    nativesdk-binutils \
    nativesdk-openocd-stm32mp \
    nativesdk-stm32wrapper4dbg \
    nativesdk-sdcard-raw-tools \
    nativesdk-ncurses-libncursesw \
    "

# For support of perl ansicolor
ST_TOOLS_FOR_SDK_PERL = " \
    nativesdk-perl-module-term-ansicolor \
    nativesdk-perl-module-encode \
    nativesdk-perl-module-encode-encoding \
    nativesdk-perl-module-encode-mime-header \
"

# For some scripts in kernel source code
ST_TOOLS_FOR_SDK_PERL:append = " \
    nativesdk-perl-module-file-spec-functions \
    nativesdk-perl-module-integer \
"

ST_TOOLS_FOR_SDK:append = " ${ST_TOOLS_FOR_SDK_PERL} "

# For support of python module for u-boot
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-python3-dev \
    nativesdk-python3-setuptools \
    nativesdk-swig \
    nativesdk-dtc \
    nativesdk-flex \
    "

# For support of python module for optee-os
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-python3-pyelftools \
    nativesdk-python3-pycryptodomex \
    nativesdk-python3-pillow \
    nativesdk-python3-cffi \
    nativesdk-python3-pycparser \
    nativesdk-python3-cryptography \
"

# For support of string convertion (iconv) in SDK
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-glibc-gconv-utf-16 \
    nativesdk-glibc-gconv-utf-32 \
    "
# For support wayland-scanner in SDK
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-wayland \
    "

# For Kernel yaml dependency
ST_TOOLS_FOR_SDK:remove = " \
    nativesdk-libyaml \
    nativesdk-libyaml-dev \
    "

# For CMSIS-SVD file parser with GDB
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-svd-tools \
    "

# For Trusted Application development using OP-TEE TA SDK
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-optee-sdk \
    "

# Fip tool
ST_TOOLS_FOR_SDK:append = " \
    nativesdk-${FIPTOOL_WRAPPER} \
    "

# Make sure to append mkimage to SDK for kernel uImage build
ST_DEPENDENCIES_BUILD_FOR_SDK = " \
    ${@bb.utils.contains('KERNEL_IMAGETYPE', 'uImage', 'nativesdk-u-boot-mkimage', '', d)} \
    ${@bb.utils.contains('KERNEL_ALT_IMAGETYPE', 'uImage', 'nativesdk-u-boot-mkimage', '', d)} \
  "
# Make sure to append openssl to SDK for kernel-module and scripts build
ST_DEPENDENCIES_BUILD_FOR_SDK:append = " nativesdk-openssl-dev "

# Make sure to append bison to SDK for u-boot build
ST_DEPENDENCIES_BUILD_FOR_SDK:append = " \
    ${@bb.utils.contains('EXTRA_IMAGEDEPENDS', 'u-boot-stm32mp', 'nativesdk-bison', '', d)} \
  "

# for populate_sdk, we will have all the tools
TOOLCHAIN_HOST_TASK:append = " ${ST_TOOLS_FOR_SDK} "
TOOLCHAIN_HOST_TASK:append = " ${ST_DEPENDENCIES_BUILD_FOR_SDK} "
# for populate_sdk_ext, the tools are not desired as mandatory tools (aka basic
# tools for devtool)
TOOLCHAIN_HOST_TASK:remove:task-populate-sdk-ext = " ${ST_TOOLS_FOR_SDK} "
TOOLCHAIN_HOST_TASK:remove:task-populate-sdk-ext = " ${ST_DEPENDENCIES_BUILD_FOR_SDK} "
# buildtools is used only by populate_sdk_ext
# populate_sdk_ext = buildtools + uninative + layer
# buildtools: sdk part of esdk (like sdk generated by populate_sdk)
# uninative: basic tools for devtool
TOOLCHAIN_HOST_TASK:remove:pn-buildtools-tarball = " ${ST_TOOLS_FOR_SDK_PERL} "

TOOLCHAIN_TARGET_TASK += " bash-dev libgomp-dev"
TOOLCHAIN_TARGET_TASK:remove:pn-buildtools-tarball = " bash-dev libgomp-dev"

# =========================================================================
# Debug trace
# =========================================================================
# acitvate/desactive the debug and trace on boot stage: tf-a, optee, u-boot and kernel
ST_DEBUG_TRACE ?= "1"

# =========================================================================
# Kernel
# =========================================================================
# Kernel image type
KERNEL_IMAGETYPE     =  "${@bb.utils.contains('MACHINE_FEATURES', 'fit', 'fitImage', 'uImage', d)}"
KERNEL_ALT_IMAGETYPE =  " Image "
KERNEL_ALT_IMAGETYPE =+ " vmlinux "
KERNEL_ALT_IMAGETYPE =+ " ${@bb.utils.contains('MACHINE_FEATURES', 'fit', 'uImage', 'zImage', d)} "

KERNEL_CLASSES = " ${@bb.utils.contains('MACHINE_FEATURES', 'fit', 'kernel-fitimage', 'kernel-uimage', d)} "

# Maxsize authorized for uncompressed kernel binary
# Define to null to skip kernel image size check
KERNEL_IMAGE_MAXSIZE ?= ""

# Enable kernel config file deploy
KERNEL_CONFIG_DEPLOY ?= "1"

# For fit usage
UBOOT_ENTRYPOINT = "0xC0800000"

# List of device tree to install
KERNEL_DEVICETREE ?= "${STM32MP_KERNEL_DEVICETREE}"
STM32MP_KERNEL_DEVICETREE += "${@' '.join('%s.dtb' % d for d in '${STM32MP_DEVICETREE}'.split())}"
STM32MP_KERNEL_DEVICETREE += "${@' '.join('%s.dtb' % d for d in '${CUBE_M4_EXAMPLES_DT}'.split())}"
STM32MP_KERNEL_DEVICETREE += "${@' '.join('%s.dtb' % d for d in '${LINUX_A7_EXAMPLES_DT}'.split())}"

# Set LOADADDR
# Set this address to 0xC2000040, which is 0xC2000000 + 0x40.
# 0xC2000000 is the memory address where U-Boot will copy from flash the file uImage and 0x40 is uImage header size (64Bytes).
# With this value, U-Boot will be able to execute in place the zImage contained in uImage.
ST_KERNEL_LOADADDR ?= "0xC2000040"

# Define the devicetree for Linux A7 examples
LINUX_A7_EXAMPLES_DT ?= ""

ST_CMD_LINE_DEBUG_TRACE ?= "loglevel=1 quiet"
# =========================================================================
# u-boot
# =========================================================================
EXTRA_IMAGEDEPENDS += "virtual/bootloader"

# Define default U-Boot config
UBOOT_CONFIG = ""

# STM32mp15
UBOOT_CONFIG:append:stm32mp15common = " ${@bb.utils.contains_any('BOOTSCHEME_LABELS', 'optee trusted', 'trusted_stm32mp15', '', d)} "
UBOOT_CONFIG:append:stm32mp15common = " trusted_stm32mp157f trusted_stm32mp151c "

# STM32mp13
UBOOT_CONFIG:append:stm32mp13common = " ${@bb.utils.contains_any('BOOTSCHEME_LABELS', 'optee trusted', 'trusted_stm32mp13', '', d)} "

# Select u-boot binary that needs specific devicetree suffix (from UBOOT_DEVICETREE)
# 'u-boot-nodtb.bin' and 'u-boot.dtb' : prefer 'u-boot.dtb' binary as 'u-boot-nodtb.bin'
# doesn't need any devicetree suffix (specific case for 'u-boot-nodtb' binary
# implemented on u-boot-stm32mp recipe)
# Define u-boot defconfig and binary to use for each UBOOT_CONFIG
UBOOT_CONFIG[trusted_stm32mp15] = "stm32mp15_defconfig,,u-boot.dtb"
UBOOT_CONFIG[trusted_stm32mp13] = "stm32mp13_defconfig,,u-boot.dtb"
UBOOT_CONFIG[trusted_stm32mp157f] = "stm32mp157f_dn_som_defconfig,,u-boot.dtb"
UBOOT_CONFIG[trusted_stm32mp151c] = "stm32mp151c_dn_som_defconfig,,u-boot.dtb"


# List of U-Boot device tree to use
UBOOT_DEVICETREE = "${STM32MP_DEVICETREE}"

# Define U-boot splashscreen file naming
UBOOT_SPLASH_PORTRAIT_IMAGE = "splash_portrait"
UBOOT_SPLASH_LANDSCAPE_IMAGE = "splash_landscape"

# Enable MTDPART check for UBOOT_CONFIG
UBOOT_MTDPART_CHECK_ENABLE = ""

# Set U-Boot MTD partition configurations
UBOOT_MTDPART_NAND_BOOT    ?= "${@','.join(['%sk(%s)' % (align_size(d, 'NAND', l.split(',')[2], l.split(',')[4]), l.split(',')[1]) for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'nand-4-256').split()])}"
UBOOT_MTDPART_NOR_BOOT     ?= "${@','.join(['%sk(%s)' % (align_size(d, 'NOR', l.split(',')[2], l.split(',')[4]), l.split(',')[1]) for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'nor-sdcard').split()])}"
UBOOT_MTDPART_SPINAND_BOOT ?= "${@','.join(['%sk(%s)' % (align_size(d, 'SPINAND', l.split(',')[2], l.split(',')[4]), l.split(',')[1]) for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'spinand-2-128').split()])}"

UBOOT_MTDPART_CHECK ?= "\
    ${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'CONFIG_MTDPARTS_NAND0_BOOT=${UBOOT_MTDPART_NAND_BOOT}', '', d)} \
    ${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor-sdcard', 'CONFIG_MTDPARTS_NOR0_BOOT=${UBOOT_MTDPART_NOR_BOOT}', '', d)} \
    ${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'CONFIG_MTDPARTS_SPINAND0_BOOT=${UBOOT_MTDPART_SPINAND_BOOT}', '', d)} \
    "

PREFERRED_PROVIDER_u-boot-fw-utils:stm32mp1common = "libubootenv"

MACHINE_EXTRA_RRECOMMENDS:append:stm32mp1common = " \
    u-boot-fw-config-stm32mp \
"

ST_UBOOT_DEBUG_TRACE ?= "${@bb.utils.contains('ST_DEBUG_TRACE', '1', '1', '0', d)}"
# =========================================================================
# trusted-firmware-a
# =========================================================================
# Add optionnaly trusted-firmware-a
EXTRA_IMAGEDEPENDS += "${@bb.utils.contains_any('BOOTSCHEME_LABELS', 'optee trusted', 'virtual/trusted-firmware-a', '', d)}"

# Configure trusted-firmware-a build
TF_A_CONFIG += "${@bb.utils.contains('BOOTSCHEME_LABELS', 'optee', 'optee', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTSCHEME_LABELS', 'trusted', 'trusted', '', d)}"
# Manage specific config settings
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'emmc', 'emmc', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'nand', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor-sdcard', 'nor', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'sdcard', 'sdcard', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'spinand', '', d)}"
TF_A_CONFIG += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor', 'nor', '', d)}"
TF_A_CONFIG += "uart"
TF_A_CONFIG += "usb"

# Append SSP config to TF_A_CONFIG
TF_A_SSP_ENABLE ?= "0"
TF_A_CONFIG += "${@bb.utils.contains('TF_A_SSP_ENABLE', '1', 'uart-ssp usb-ssp', '', d)}"

# Default configuration for signing trusted-firmware-a binary
TF_A_SIGN_ENABLE ?= "0"

# Configure TF-A to build the metadata binary
TF_A_ENABLE_METADATA ?= "${@bb.utils.contains('MACHINE_FEATURES', 'fw-update', '1', '0', d)}"
TF_A_METADATA_BINARY ?= "metadata.bin"

# Configure the default MTD_START_OFFSET
TF_A_MTD_START_OFFSET_NAND    ?= "0x00200000"
TF_A_MTD_START_OFFSET_NOR     ?= "${@bb.utils.contains('MACHINE_FEATURES', 'fw-update', '0x00080000', '0x00100000', d)}"
TF_A_MTD_START_OFFSET_SPINAND ?= "0x00200000"

ST_TF_A_DEBUG_TRACE ?= "${@bb.utils.contains('ST_DEBUG_TRACE', '1', '1', '0', d)}"
# =========================================================================
# optee
# =========================================================================
# Map OPTEE configuration to device tree list
OPTEE_CONF = "${STM32MP_DEVICETREE}"

ST_OPTEE_DEBUG_TRACE ?= "${@bb.utils.contains('ST_DEBUG_TRACE', '1', '1', '0', d)}"
# =========================================================================
# fip
# =========================================================================
# Configure fip build
FIP_CONFIG += "${@bb.utils.contains('BOOTSCHEME_LABELS', 'optee', 'optee', '', d)}"
FIP_CONFIG += "${@bb.utils.contains('BOOTSCHEME_LABELS', 'trusted', 'trusted', '', d)}"

# Define config for each FIP_CONFIG
FIP_CONFIG[optee]   ?= "optee,${FIP_DEVICETREE_OPTEE}"
FIP_CONFIG[trusted] ?= "tfa,${FIP_DEVICETREE_TRUSTED}"

# List of device tree to use for fip binary creation
FIP_DEVICETREE_OPTEE   ?= "${STM32MP_DEVICETREE}"
FIP_DEVICETREE_TRUSTED ?= "${@' '.join(d for d in '${STM32MP_DEVICETREE}'.split() if 'stm32mp15' in d)}"

# Define fiptool wrapper name
FIPTOOL_WRAPPER ?= "fiptool-stm32mp"

# Manage specific config to sign FIP
FIP_SIGN_ENABLE ?= "${@bb.utils.contains('TF_A_SIGN_ENABLE', '1', '1', '', d)}"
FIP_SIGN_SUFFIX ?= "${@bb.utils.contains('FIP_SIGN_ENABLE', '1', '_Signed', '', d)}"

# =========================================================================
# Xserver
# =========================================================================
#XSERVER ?= " \
#    xserver-xorg \
#    xserver-xorg-module-libint10 \
#    xf86-input-evdev \
#    xf86-video-modesetting \
#"

# =========================================================================
# Enable deploy of bootloader elf files
# =========================================================================
ELF_DEBUG_ENABLE = "1"

# =========================================================================
# sysdig
# =========================================================================
PREFERRED_PROVIDER_sysdig = "sysdig-stm32mp"

# =========================================================================
# M4 copro
# =========================================================================
# Define the devicetree for M4 examples
CUBE_M4_EXAMPLES_DT ?= ""

# Define the name of default copro firmware executed @boot time
# This name is cherry picked from list defined in m4projects-stm32mp1.bb
DEFAULT_COPRO_FIRMWARE = "OpenAMP_TTY_echo"

# Define M4 example installation dir
M4_INSTALLDIR = "${STM32MP_USERFS_MOUNTPOINT}"
M4_PACKAGE_4USERFS = "${@bb.utils.contains('ST_USERFS','1','1','0',d)}"

# =========================================================================
# GCNANO userland configuration
# =========================================================================
# Configure use of vendorfs if enable to install gcnano libs
GCNANO_USERLAND_OUTPUT_LIBDIR = "${@bb.utils.contains('ST_VENDORFS', '1', '${STM32MP_VENDORFS_MOUNTPOINT}/lib', '${libdir}', d)}"
