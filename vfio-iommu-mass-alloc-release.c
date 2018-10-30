/*
 * VFIO API definition
 *
 * Copyright (C) 2012 Red Hat, Inc.  All rights reserved.
 *     Author: Alex Williamson <alex.williamson@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _UAPIVFIO_H
#define _UAPIVFIO_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define VFIO_API_VERSION    0


/* Kernel & User level defines for VFIO IOCTLs. */

/* Extensions */

#define VFIO_TYPE1_IOMMU        1
#define VFIO_NOIOMMU_IOMMU              8

/*
 * The IOCTL interface is designed for extensibility by embedding the
 * structure length (argsz) and flags into structures passed between
 * kernel and userspace.  We therefore use the _IO() macro for these
 * defines to avoid implicitly embedding a size into the ioctl request.
 * As structure fields are added, argsz will increase to match and flag
 * bits will be defined to indicate additional fields with valid data.
 * It's *always* the caller's responsibility to indicate the size of
 * the structure passed by setting argsz appropriately.
 */

#define VFIO_TYPE    (';')
#define VFIO_BASE    100

struct vfio_info_cap_header {
        __u16   id;             /* Identifies capability */
        __u16   version;        /* Version specific to the capability ID */
        __u32   next;           /* Offset of next capability */
};

/* -------- IOCTLs for VFIO file descriptor (/dev/vfio/vfio) -------- */

/**
 * VFIO_GET_API_VERSION - _IO(VFIO_TYPE, VFIO_BASE + 0)
 *
 * Report the version of the VFIO API.  This allows us to bump the entire
 * API version should we later need to add or change features in incompatible
 * ways.
 * Return: VFIO_API_VERSION
 * Availability: Always
 */
#define VFIO_GET_API_VERSION        _IO(VFIO_TYPE, VFIO_BASE + 0)

/**
 * VFIO_CHECK_EXTENSION - _IOW(VFIO_TYPE, VFIO_BASE + 1, __u32)
 *
 * Check whether an extension is supported.
 * Return: 0 if not supported, 1 (or some other positive integer) if supported.
 * Availability: Always
 */
#define VFIO_CHECK_EXTENSION        _IO(VFIO_TYPE, VFIO_BASE + 1)

/**
 * VFIO_SET_IOMMU - _IOW(VFIO_TYPE, VFIO_BASE + 2, __s32)
 *
 * Set the iommu to the given type.  The type must be supported by an
 * iommu driver as verified by calling CHECK_EXTENSION using the same
 * type.  A group must be set to this file descriptor before this
 * ioctl is available.  The IOMMU interfaces enabled by this call are
 * specific to the value set.
 * Return: 0 on success, -errno on failure
 * Availability: When VFIO group attached
 */
#define VFIO_SET_IOMMU            _IO(VFIO_TYPE, VFIO_BASE + 2)

/* -------- IOCTLs for GROUP file descriptors (/dev/vfio/$GROUP) -------- */

/**
 * VFIO_GROUP_GET_STATUS - _IOR(VFIO_TYPE, VFIO_BASE + 3,
 *                        struct vfio_group_status)
 *
 * Retrieve information about the group.  Fills in provided
 * struct vfio_group_info.  Caller sets argsz.
 * Return: 0 on succes, -errno on failure.
 * Availability: Always
 */
struct vfio_group_status {
    __u32    argsz;
    __u32    flags;
#define VFIO_GROUP_FLAGS_VIABLE        (1 << 0)
#define VFIO_GROUP_FLAGS_CONTAINER_SET    (1 << 1)
};
#define VFIO_GROUP_GET_STATUS        _IO(VFIO_TYPE, VFIO_BASE + 3)

/**
 * VFIO_GROUP_SET_CONTAINER - _IOW(VFIO_TYPE, VFIO_BASE + 4, __s32)
 *
 * Set the container for the VFIO group to the open VFIO file
 * descriptor provided.  Groups may only belong to a single
 * container.  Containers may, at their discretion, support multiple
 * groups.  Only when a container is set are all of the interfaces
 * of the VFIO file descriptor and the VFIO group file descriptor
 * available to the user.
 * Return: 0 on success, -errno on failure.
 * Availability: Always
 */
#define VFIO_GROUP_SET_CONTAINER    _IO(VFIO_TYPE, VFIO_BASE + 4)

/**
 * VFIO_GROUP_UNSET_CONTAINER - _IO(VFIO_TYPE, VFIO_BASE + 5)
 *
 * Remove the group from the attached container.  This is the
 * opposite of the SET_CONTAINER call and returns the group to
 * an initial state.  All device file descriptors must be released
 * prior to calling this interface.  When removing the last group
 * from a container, the IOMMU will be disabled and all state lost,
 * effectively also returning the VFIO file descriptor to an initial
 * state.
 * Return: 0 on success, -errno on failure.
 * Availability: When attached to container
 */
#define VFIO_GROUP_UNSET_CONTAINER    _IO(VFIO_TYPE, VFIO_BASE + 5)

/**
 * VFIO_GROUP_GET_DEVICE_FD - _IOW(VFIO_TYPE, VFIO_BASE + 6, char)
 *
 * Return a new file descriptor for the device object described by
 * the provided string.  The string should match a device listed in
 * the devices subdirectory of the IOMMU group sysfs entry.  The
 * group containing the device must already be added to this context.
 * Return: new file descriptor on success, -errno on failure.
 * Availability: When attached to container
 */
#define VFIO_GROUP_GET_DEVICE_FD    _IO(VFIO_TYPE, VFIO_BASE + 6)

/* --------------- IOCTLs for DEVICE file descriptors --------------- */

/**
 * VFIO_DEVICE_GET_INFO - _IOR(VFIO_TYPE, VFIO_BASE + 7,
 *                        struct vfio_device_info)
 *
 * Retrieve information about the device.  Fills in provided
 * struct vfio_device_info.  Caller sets argsz.
 * Return: 0 on success, -errno on failure.
 */
struct vfio_device_info {
    __u32    argsz;
    __u32    flags;
#define VFIO_DEVICE_FLAGS_RESET    (1 << 0)    /* Device supports reset */
#define VFIO_DEVICE_FLAGS_PCI    (1 << 1)    /* vfio-pci device */
    __u32    num_regions;    /* Max region index + 1 */
    __u32    num_irqs;    /* Max IRQ index + 1 */
};
#define VFIO_DEVICE_GET_INFO        _IO(VFIO_TYPE, VFIO_BASE + 7)

/**
 * VFIO_DEVICE_GET_REGION_INFO - _IOWR(VFIO_TYPE, VFIO_BASE + 8,
 *                       struct vfio_region_info)
 *
 * Retrieve information about a device region.  Caller provides
 * struct vfio_region_info with index value set.  Caller sets argsz.
 * Implementation of region mapping is bus driver specific.  This is
 * intended to describe MMIO, I/O port, as well as bus specific
 * regions (ex. PCI config space).  Zero sized regions may be used
 * to describe unimplemented regions (ex. unimplemented PCI BARs).
 * Return: 0 on success, -errno on failure.
 */
struct vfio_region_info {
    __u32    argsz;
    __u32    flags;
#define VFIO_REGION_INFO_FLAG_READ    (1 << 0) /* Region supports read */
#define VFIO_REGION_INFO_FLAG_WRITE    (1 << 1) /* Region supports write */
#define VFIO_REGION_INFO_FLAG_MMAP    (1 << 2) /* Region supports mmap */
#define VFIO_REGION_INFO_FLAG_CAPS      (1 << 3) /* Info supports caps */
    __u32    index;        /* Region index */
    __u32    cap_offset;    /* Offset within info struct of first cap */
    __u64    size;        /* Region size (bytes) */
    __u64    offset;        /* Region offset from start of device fd */
};
#define VFIO_DEVICE_GET_REGION_INFO    _IO(VFIO_TYPE, VFIO_BASE + 8)

#define VFIO_REGION_INFO_CAP_SPARSE_MMAP        1

struct vfio_region_sparse_mmap_area {
        __u64   offset; /* Offset of mmap'able area within region */
        __u64   size;   /* Size of mmap'able area */
};

struct vfio_region_info_cap_sparse_mmap {
        struct vfio_info_cap_header header;
        __u32   nr_areas;
        __u32   reserved;
        struct vfio_region_sparse_mmap_area areas[];
};

#define VFIO_REGION_INFO_CAP_TYPE       2

struct vfio_region_info_cap_type {
        struct vfio_info_cap_header header;
        __u32 type;     /* global per bus driver */
        __u32 subtype;  /* type specific */
};

#define VFIO_REGION_TYPE_PCI_VENDOR_TYPE        (1 << 31)
#define VFIO_REGION_TYPE_PCI_VENDOR_MASK        (0xffff)

#define VFIO_REGION_SUBTYPE_INTEL_IGD_OPREGION  (1)

/**
 * VFIO_DEVICE_GET_IRQ_INFO - _IOWR(VFIO_TYPE, VFIO_BASE + 9,
 *                    struct vfio_irq_info)
 *
 * Retrieve information about a device IRQ.  Caller provides
 * struct vfio_irq_info with index value set.  Caller sets argsz.
 * Implementation of IRQ mapping is bus driver specific.  Indexes
 * using multiple IRQs are primarily intended to support MSI-like
 * interrupt blocks.  Zero count irq blocks may be used to describe
 * unimplemented interrupt types.
 *
 * The EVENTFD flag indicates the interrupt index supports eventfd based
 * signaling.
 *
 * The MASKABLE flags indicates the index supports MASK and UNMASK
 * actions described below.
 *
 * AUTOMASKED indicates that after signaling, the interrupt line is
 * automatically masked by VFIO and the user needs to unmask the line
 * to receive new interrupts.  This is primarily intended to distinguish
 * level triggered interrupts.
 *
 * The NORESIZE flag indicates that the interrupt lines within the index
 * are setup as a set and new subindexes cannot be enabled without first
 * disabling the entire index.  This is used for interrupts like PCI MSI
 * and MSI-X where the driver may only use a subset of the available
 * indexes, but VFIO needs to enable a specific number of vectors
 * upfront.  In the case of MSI-X, where the user can enable MSI-X and
 * then add and unmask vectors, it's up to userspace to make the decision
 * whether to allocate the maximum supported number of vectors or tear
 * down setup and incrementally increase the vectors as each is enabled.
 */
struct vfio_irq_info {
    __u32    argsz;
    __u32    flags;
#define VFIO_IRQ_INFO_EVENTFD        (1 << 0)
#define VFIO_IRQ_INFO_MASKABLE        (1 << 1)
#define VFIO_IRQ_INFO_AUTOMASKED    (1 << 2)
#define VFIO_IRQ_INFO_NORESIZE        (1 << 3)
    __u32    index;        /* IRQ index */
    __u32    count;        /* Number of IRQs within this index */
};
#define VFIO_DEVICE_GET_IRQ_INFO    _IO(VFIO_TYPE, VFIO_BASE + 9)

/**
 * VFIO_DEVICE_SET_IRQS - _IOW(VFIO_TYPE, VFIO_BASE + 10, struct vfio_irq_set)
 *
 * Set signaling, masking, and unmasking of interrupts.  Caller provides
 * struct vfio_irq_set with all fields set.  'start' and 'count' indicate
 * the range of subindexes being specified.
 *
 * The DATA flags specify the type of data provided.  If DATA_NONE, the
 * operation performs the specified action immediately on the specified
 * interrupt(s).  For example, to unmask AUTOMASKED interrupt [0,0]:
 * flags = (DATA_NONE|ACTION_UNMASK), index = 0, start = 0, count = 1.
 *
 * DATA_BOOL allows sparse support for the same on arrays of interrupts.
 * For example, to mask interrupts [0,1] and [0,3] (but not [0,2]):
 * flags = (DATA_BOOL|ACTION_MASK), index = 0, start = 1, count = 3,
 * data = {1,0,1}
 *
 * DATA_EVENTFD binds the specified ACTION to the provided __s32 eventfd.
 * A value of -1 can be used to either de-assign interrupts if already
 * assigned or skip un-assigned interrupts.  For example, to set an eventfd
 * to be trigger for interrupts [0,0] and [0,2]:
 * flags = (DATA_EVENTFD|ACTION_TRIGGER), index = 0, start = 0, count = 3,
 * data = {fd1, -1, fd2}
 * If index [0,1] is previously set, two count = 1 ioctls calls would be
 * required to set [0,0] and [0,2] without changing [0,1].
 *
 * Once a signaling mechanism is set, DATA_BOOL or DATA_NONE can be used
 * with ACTION_TRIGGER to perform kernel level interrupt loopback testing
 * from userspace (ie. simulate hardware triggering).
 *
 * Setting of an event triggering mechanism to userspace for ACTION_TRIGGER
 * enables the interrupt index for the device.  Individual subindex interrupts
 * can be disabled using the -1 value for DATA_EVENTFD or the index can be
 * disabled as a whole with: flags = (DATA_NONE|ACTION_TRIGGER), count = 0.
 *
 * Note that ACTION_[UN]MASK specify user->kernel signaling (irqfds) while
 * ACTION_TRIGGER specifies kernel->user signaling.
 */
struct vfio_irq_set {
    __u32    argsz;
    __u32    flags;
#define VFIO_IRQ_SET_DATA_NONE        (1 << 0) /* Data not present */
#define VFIO_IRQ_SET_DATA_BOOL        (1 << 1) /* Data is bool (u8) */
#define VFIO_IRQ_SET_DATA_EVENTFD    (1 << 2) /* Data is eventfd (s32) */
#define VFIO_IRQ_SET_ACTION_MASK    (1 << 3) /* Mask interrupt */
#define VFIO_IRQ_SET_ACTION_UNMASK    (1 << 4) /* Unmask interrupt */
#define VFIO_IRQ_SET_ACTION_TRIGGER    (1 << 5) /* Trigger interrupt */
    __u32    index;
    __u32    start;
    __u32    count;
    __u8    data[];
};
#define VFIO_DEVICE_SET_IRQS        _IO(VFIO_TYPE, VFIO_BASE + 10)

#define VFIO_IRQ_SET_DATA_TYPE_MASK    (VFIO_IRQ_SET_DATA_NONE | \
                     VFIO_IRQ_SET_DATA_BOOL | \
                     VFIO_IRQ_SET_DATA_EVENTFD)
#define VFIO_IRQ_SET_ACTION_TYPE_MASK    (VFIO_IRQ_SET_ACTION_MASK | \
                     VFIO_IRQ_SET_ACTION_UNMASK | \
                     VFIO_IRQ_SET_ACTION_TRIGGER)
/**
 * VFIO_DEVICE_RESET - _IO(VFIO_TYPE, VFIO_BASE + 11)
 *
 * Reset a device.
 */
#define VFIO_DEVICE_RESET        _IO(VFIO_TYPE, VFIO_BASE + 11)

/*
 * The VFIO-PCI bus driver makes use of the following fixed region and
 * IRQ index mapping.  Unimplemented regions return a size of zero.
 * Unimplemented IRQ types return a count of zero.
 */

enum {
    VFIO_PCI_BAR0_REGION_INDEX,
    VFIO_PCI_BAR1_REGION_INDEX,
    VFIO_PCI_BAR2_REGION_INDEX,
    VFIO_PCI_BAR3_REGION_INDEX,
    VFIO_PCI_BAR4_REGION_INDEX,
    VFIO_PCI_BAR5_REGION_INDEX,
    VFIO_PCI_ROM_REGION_INDEX,
    VFIO_PCI_CONFIG_REGION_INDEX,
    /*
     * Expose VGA regions defined for PCI base class 03, subclass 00.
     * This includes I/O port ranges 0x3b0 to 0x3bb and 0x3c0 to 0x3df
     * as well as the MMIO range 0xa0000 to 0xbffff.  Each implemented
     * range is found at it's identity mapped offset from the region
     * offset, for example 0x3b0 is region_info.offset + 0x3b0.  Areas
     * between described ranges are unimplemented.
     */
    VFIO_PCI_VGA_REGION_INDEX,
    VFIO_PCI_NUM_REGIONS
};

enum {
    VFIO_PCI_INTX_IRQ_INDEX,
    VFIO_PCI_MSI_IRQ_INDEX,
    VFIO_PCI_MSIX_IRQ_INDEX,
    VFIO_PCI_NUM_IRQS
};

/**
 * VFIO_DEVICE_GET_PCI_HOT_RESET_INFO - _IORW(VFIO_TYPE, VFIO_BASE + 12,
 *                                            struct vfio_pci_hot_reset_info)
 *
 * Return: 0 on success, -errno on failure:
 *      -enospc = insufficient buffer, -enodev = unsupported for device.
 */
struct vfio_pci_dependent_device {
        __u32   group_id;
        __u16   segment;
        __u8    bus;
        __u8    devfn; /* Use PCI_SLOT/PCI_FUNC */
};

struct vfio_pci_hot_reset_info {
        __u32   argsz;
        __u32   flags;
        __u32   count;
        struct vfio_pci_dependent_device        devices[];
};

#define VFIO_DEVICE_GET_PCI_HOT_RESET_INFO      _IO(VFIO_TYPE, VFIO_BASE + 12)

/**
 * VFIO_DEVICE_PCI_HOT_RESET - _IOW(VFIO_TYPE, VFIO_BASE + 13,
 *                                  struct vfio_pci_hot_reset)
 *
 * Return: 0 on success, -errno on failure.
 */
struct vfio_pci_hot_reset {
        __u32   argsz;
        __u32   flags;
        __u32   count;
        __s32   group_fds[];
};

#define VFIO_DEVICE_PCI_HOT_RESET       _IO(VFIO_TYPE, VFIO_BASE + 13)

/* -------- API for Type1 VFIO IOMMU -------- */

/**
 * VFIO_IOMMU_GET_INFO - _IOR(VFIO_TYPE, VFIO_BASE + 12, struct vfio_iommu_info)
 *
 * Retrieve information about the IOMMU object. Fills in provided
 * struct vfio_iommu_info. Caller sets argsz.
 *
 * XXX Should we do these by CHECK_EXTENSION too?
 */
struct vfio_iommu_type1_info {
    __u32    argsz;
    __u32    flags;
#define VFIO_IOMMU_INFO_PGSIZES (1 << 0)    /* supported page sizes info */
    __u64    iova_pgsizes;        /* Bitmap of supported page sizes */
};

#define VFIO_IOMMU_GET_INFO _IO(VFIO_TYPE, VFIO_BASE + 12)

/**
 * VFIO_IOMMU_MAP_DMA - _IOW(VFIO_TYPE, VFIO_BASE + 13, struct vfio_dma_map)
 *
 * Map process virtual addresses to IO virtual addresses using the
 * provided struct vfio_dma_map. Caller sets argsz. READ &/ WRITE required.
 */
struct vfio_iommu_type1_dma_map {
    __u32    argsz;
    __u32    flags;
#define VFIO_DMA_MAP_FLAG_READ (1 << 0)        /* readable from device */
#define VFIO_DMA_MAP_FLAG_WRITE (1 << 1)    /* writable from device */
    __u64    vaddr;                /* Process virtual address */
    __u64    iova;                /* IO virtual address */
    __u64    size;                /* Size of mapping (bytes) */
};

#define VFIO_IOMMU_MAP_DMA _IO(VFIO_TYPE, VFIO_BASE + 13)

/**
 * VFIO_IOMMU_UNMAP_DMA - _IOWR(VFIO_TYPE, VFIO_BASE + 14,
 *                            struct vfio_dma_unmap)
 *
 * Unmap IO virtual addresses using the provided struct vfio_dma_unmap.
 * Caller sets argsz.  The actual unmapped size is returned in the size
 * field.  No guarantee is made to the user that arbitrary unmaps of iova
 * or size different from those used in the original mapping call will
 * succeed.
 */
struct vfio_iommu_type1_dma_unmap {
    __u32    argsz;
    __u32    flags;
    __u64    iova;                /* IO virtual address */
    __u64    size;                /* Size of mapping (bytes) */
};

#define VFIO_IOMMU_UNMAP_DMA _IO(VFIO_TYPE, VFIO_BASE + 14)

#endif /* _UAPIVFIO_H */

#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>

#include <linux/ioctl.h>

void usage(char *name)
{
    printf("usage: %s <mode> <ssss:bb:dd.f> [...]\n", name);
    printf("  mode controlls what is done after a key is pressed:\n");
    printf("    e - just exit (return from main)\n");
    printf("    c - close device FDs before exit\n");
    printf("    t - concurrently close device FDs before exit\n");
    printf("    p - open and close in indivual process context\n");
}

void *thread_close_device(void *device)
{
    int *devicefd = (int *)device;
    close(*devicefd);
    return NULL;
}

#define MAX_DEVS 64

int main(int argc, char **argv)
{
    int i, ret, container, group[MAX_DEVS], device[MAX_DEVS], groupid;
    pthread_t thread[MAX_DEVS];
    char path[PATH_MAX], iommu_group_path[PATH_MAX], *group_name;
    struct stat st;
    ssize_t len;
    enum mode { mode_exit, mode_close, mode_thread_close, mode_process_close } mode;
    struct timespec ts[MAX_DEVS];
    int seg[MAX_DEVS], bus[MAX_DEVS], slot[MAX_DEVS], func[MAX_DEVS];

    struct vfio_group_status group_status = {
        .argsz = sizeof(group_status)
    };

    struct vfio_device_info device_info = {
        .argsz = sizeof(device_info)
    };

    switch (argv[1][0])  {
        case 'e':
            mode = mode_exit;
            break;
        case 'c':
            mode = mode_close;
            break;
        case 't':
            mode = mode_thread_close;
            break;
        case 'p':
            mode = mode_process_close;
            break;
        default:
            printf("Bad mode '%c'\n", argv[1][0]);
            usage(argv[0]);
            return -1;
            break;
    }

    struct vfio_region_info region_info = {
        .argsz = sizeof(region_info)
    };

    for (i=2;i<argc; i++) {
        ret = sscanf(argv[i], "%04x:%02x:%02x.%d", &seg[i], &bus[i], &slot[i], &func[i]);
        if (ret != 4) {
            ret = sscanf(argv[i], "%02x", &bus[i]);
            if (ret == 1) {
                seg[i] = slot[i] = func[i] = 0;
                continue;
            }
            usage(argv[0]);
            return -1;
        }
    }

    container = open("/dev/vfio/vfio", O_RDWR);
    if (container < 0) {
        printf("Failed to open /dev/vfio/vfio, %d (%s)\n",
               container, strerror(errno));
        return container;
    }
    printf("Container '%d' opened\n", container);

    for (i=2;i<argc; i++) {
        snprintf(path, sizeof(path),
             "/sys/bus/pci/devices/%04x:%02x:%02x.%01x/",
             seg[i], bus[i], slot[i], func[i]);

        ret = stat(path, &st);
        if (ret < 0) {
            printf("No such device\n");
            return  ret;
        }

        strncat(path, "iommu_group", sizeof(path) - strlen(path) - 1);

        len = readlink(path, iommu_group_path, sizeof(iommu_group_path));
        if (len <= 0) {
            printf("No iommu_group for device (%s)\n", path);
            return -1;
        }

        iommu_group_path[len] = 0;
        group_name = basename(iommu_group_path);

        if (sscanf(group_name, "%d", &groupid) != 1) {
            printf("Unknown group\n");
            return -1;
        }

        printf("Assigning group '%d' to container '%d'\n",  groupid, container);

        snprintf(path, sizeof(path), "/dev/vfio/%d", groupid);
        group[i] = open(path, O_RDWR);
        if (group[i] < 0) {
            printf("Failed to open %s, %d (%s)\n",
                   path, group[i], strerror(errno));
            return -1;
        }

        ret = ioctl(group[i], VFIO_GROUP_GET_STATUS, &group_status);
        if (ret) {
            printf("ioctl(VFIO_GROUP_GET_STATUS) failed\n");
            return ret;
        }

        if (!(group_status.flags & VFIO_GROUP_FLAGS_VIABLE)) {
            printf("Group not viable, are all devices attached to vfio?\n");
            return -1;
        }

        ret = ioctl(group[i], VFIO_GROUP_SET_CONTAINER, &container);
        if (ret) {
            printf("Failed to set group container\n");
            return ret;
        }
    }
    printf("Done: associating container with vfio groups\n");

    printf("VFIO_CHECK_EXTENSION VFIO_TYPE1_IOMMU: %sPresent\n",
           ioctl(container, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU) ?
           "" : "Not ");
    ret = ioctl(container, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU);
    if (ret) {
        printf("Failed to set IOMMU\n");
        return ret;
    }
    printf("Done: set IOMMU\n");

    if ( mode == mode_process_close) {
        sem_t *sem_ready[MAX_DEVS];
        sem_t *sem_go[MAX_DEVS];

        for (i=2;i<argc; i++) {
            snprintf(path, sizeof(path), "%04x:%02x:%02x.%d-ready",
                     seg[i], bus[i], slot[i], func[i]);
            sem_ready[i] = sem_open(path, O_CREAT | O_EXCL, 0644, 0);
            if (sem_ready[i] == SEM_FAILED) {
                fprintf(stderr, "sem_open() failed.  errno:%d\n", errno);
                exit(1);
            }
            sem_unlink(path);
            snprintf(path, sizeof(path), "%04x:%02x:%02x.%d-go",
                     seg[i], bus[i], slot[i], func[i]);
            sem_go[i] = sem_open(path, O_CREAT | O_EXCL, 0644, 0);
            if (sem_go[i] == SEM_FAILED) {
                fprintf(stderr, "sem_open() failed.  errno:%d\n", errno);
                exit(1);
            }
            sem_unlink(path);
        }

        /* open and close paths for devices are different for mode_process_close */
        for (i=2;i<argc; i++) {
            if (fork() == 0) {
                // Child
                snprintf(path, sizeof(path), "%04x:%02x:%02x.%d",
                         seg[i], bus[i], slot[i], func[i]);
                printf("%d: opening PCI device %s on group %d\n", i, path, group[i]);

                device[i] = ioctl(group[i], VFIO_GROUP_GET_DEVICE_FD, path);
                if (device[i] < 0) {
                    printf("Failed to get device %s\n", path);
                    return -1;
                }

                if (ioctl(device[i], VFIO_DEVICE_GET_INFO, &device_info)) {
                    printf("Failed to get device info\n");
                    return -1;
                }

                sem_post(sem_ready[i]);
                sem_wait(sem_go[i]);
                // exit would release the vfio FD anyway, but be explicit to be sure on the timing
                clock_gettime(CLOCK_MONOTONIC, &ts[i]);
                printf("%d: start close - %ld.%09ld\n", i, ts[i].tv_sec, ts[i].tv_nsec);
                close(device[i]);
                clock_gettime(CLOCK_MONOTONIC, &ts[i]);
                printf("%d: close done - %ld.%09ld\n", i, ts[i].tv_sec, ts[i].tv_nsec);
                sem_post(sem_ready[i]);
                sem_wait(sem_go[i]);
                printf("Exit child %d\n", i);
                sem_close(sem_ready[i]);
                sem_close(sem_go[i]);
                exit(0);
            }
        }
        // Parent after all childs are forked
        for (i=2;i<argc; i++) {
            sem_wait(sem_ready[i]);
        }
        printf("All devices attached to childs - Press any key to exit\n");
        fgetc(stdin);

        clock_gettime(CLOCK_MONOTONIC, &ts[0]);
        for (i=2;i<argc; i++) {
            clock_gettime(CLOCK_MONOTONIC, &ts[i]);
            printf("release %d - %ld.%09ld\n", i, ts[i].tv_sec, ts[i].tv_nsec);
            sem_post(sem_go[i]);
        }
        printf("Started concurrent exit in childs at - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);
        for (i=2;i<argc; i++) {
            sem_wait(sem_ready[i]);
        }
        clock_gettime(CLOCK_MONOTONIC, &ts[0]);
        printf("All childs dropped vfio device - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);

        printf("Press any key to exit\n");
        fgetc(stdin);
        for (i=2;i<argc; i++) {
            sem_post(sem_go[i]);
        }
        for (i=2;i<argc; i++) {
            sem_close(sem_ready[i]);
            sem_close(sem_go[i]);
        }
        printf("Exit parent\n");
        exit(0);
    }
    else {
        for (i=2;i<argc; i++) {
            snprintf(path, sizeof(path), "%04x:%02x:%02x.%d",
                     seg[i], bus[i], slot[i], func[i]);
            printf("opening PCI device %s on group %d\n", path, group[i]);

            device[i] = ioctl(group[i], VFIO_GROUP_GET_DEVICE_FD, path);
            if (device[i] < 0) {
                printf("Failed to get device %s\n", path);
                return -1;
            }

            if (ioctl(device[i], VFIO_DEVICE_GET_INFO, &device_info)) {
                printf("Failed to get device info\n");
                return -1;
            }

            printf("Device (%d) supports %d regions, %d irqs\n",
                   device[i], device_info.num_regions, device_info.num_irqs);
        }

        printf("All devices attached - Press any key to exit\n");
        fgetc(stdin);

        clock_gettime(CLOCK_MONOTONIC, &ts[0]);
        printf("Start exit path - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);

        if ( mode == mode_close) {
            printf("Closing devices in a sequential loop\n");
            for (i=2;i<argc; i++) {
                clock_gettime(CLOCK_MONOTONIC, &ts[i]);
                printf("Close device '%d' - %ld.%09ld\n", device[i], ts[i].tv_sec, ts[i].tv_nsec);
                close(device[i]);
            }
            clock_gettime(CLOCK_MONOTONIC, &ts[0]);
            printf("Droped all devices - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);
            printf("Press any key to exit\n");
            fgetc(stdin);
        }

        if ( mode == mode_thread_close) {
            clock_gettime(CLOCK_MONOTONIC, &ts[0]);
            printf("Closing devices concurrently - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);
            for (i=2;i<argc; i++) {
                if(pthread_create(&thread[i], NULL, thread_close_device, &device[i])) {
                    printf("Error creating thread\n");
                    return -1;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &ts[0]);
            printf("Spawned all closing threads - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);
            for (i=2;i<argc; i++) {
                if(pthread_join(thread[i], NULL)) {
                    printf("Error joining thread\n");
                    return -1;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &ts[0]);
            printf("Droped all devices - %ld.%09ld\n", ts[0].tv_sec, ts[0].tv_nsec);
            printf("Press any key to exit\n");
            fgetc(stdin);
        }
    }

    return 0;
}
