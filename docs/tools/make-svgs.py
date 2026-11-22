import sys
from xml.sax.saxutils import escape

STROKE = '#8a8a8a'
TEXT = '#8a8a8a'
KINDS = {
    'open': ('#3c9a5f', '0.22', None),
    'ours': ('#4a7fd9', '0.28', None),
    'closed': ('#d9534f', '0.20', '5 3'),
    'none': ('none', '0', None),
}


def box(x, y, w, h, title, driver, kind):
    fill, opacity, dash = KINDS[kind]
    dash_attr = f' stroke-dasharray="{dash}"' if dash else ''
    out = [f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="{fill}" fill-opacity="{opacity}" stroke="{STROKE}" stroke-width="1.5"{dash_attr}/>']
    cx = x + w / 2
    if driver:
        out.append(f'<text x="{cx}" y="{y + h / 2 - 4}" text-anchor="middle" font-size="12.5">{escape(title)}</text>')
        out.append(f'<text x="{cx}" y="{y + h / 2 + 13}" text-anchor="middle" font-size="11"{' font-family="Menlo, Consolas, monospace"' if kind != 'none' else ''}>{escape(driver)}</text>')
    else:
        out.append(f'<text x="{cx}" y="{y + h / 2 + 4}" text-anchor="middle" font-size="12.5">{escape(title)}</text>')
    return out


def container(x, y, w, h, label):
    return [f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="6" fill="none" stroke="{STROKE}" stroke-width="1.2"/>',
            f'<text x="{x + 10}" y="{y + 17}" font-size="12">{escape(label)}</text>']


def line(x1, y1, x2, y2, label=None):
    out = [f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{STROKE}" stroke-width="1.5"/>']
    if label:
        out.append(f'<text x="{(x1 + x2) / 2 + 6}" y="{(y1 + y2) / 2 + 4}" font-size="11">{escape(label)}</text>')
    return out


def legend(y, width):
    items = [('open', 'built here from Apple\u2019s source'), ('ours', 'written by this project'), ('closed', 'Apple\u2019s closed driver, not on the image'), ('none', 'no driver of its own')]
    out = []
    for i, (kind, text) in enumerate(items):
        x = 20 + (i % 2) * 330
        yy = y + (i // 2) * 24
        out += box(x, yy, 22, 16, '', '', kind)[:1]
        out.append(f'<text x="{x + 30}" y="{yy + 13}" font-size="11.5">{escape(text)}</text>')
    return out


def svg(width, height, title, parts):
    head = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" font-family="Helvetica, Arial, sans-serif">\n<title>{escape(title)}</title>\n<g fill="{TEXT}">\n'
    return head + '\n'.join(parts) + '\n</g>\n</svg>\n'


def mac99():
    p = []
    p.append('<text x="380" y="24" text-anchor="middle" font-size="15">QEMU’s mac99 (PowerMac3,1), as boot-qemu starts it</text>')
    p += box(280, 40, 200, 50, 'PowerPC G4 (7400)', 'AppleMacRISC2PE', 'open')
    p += line(380, 90, 380, 115)
    p += box(280, 115, 200, 50, 'UniNorth host bridges', 'AppleMacRiscPCI', 'open')
    p += line(480, 140, 540, 140)
    p += box(540, 115, 190, 50, 'Memory, 1 GB', '', 'none')
    p += line(380, 165, 380, 195)
    p += line(30, 195, 730, 195, None)
    p.append('<text x="36" y="188" font-size="11">PCI bus</text>')
    for x in (105, 285, 465, 645):
        p += line(x, 195, x, 215)
    p += box(30, 215, 150, 50, 'VGA display', 'firmware’s framebuffer', 'none')
    p += box(195, 215, 180, 50, 'USB OHCI: keyboard', 'IOUSBFamily, IOHIDFamily', 'open')
    p += box(390, 215, 150, 50, 'sungem Ethernet', 'AppleGMACEthernet', 'open')
    p += box(560, 215, 170, 50, 'KeyLargo mac-io', 'AppleKeyLargo', 'open')
    p += line(645, 265, 645, 290)
    p += container(20, 290, 720, 160, 'Inside KeyLargo')
    p += box(35, 318, 150, 50, 'OpenPIC: interrupts', 'AppleMPIC', 'open')
    p += box(200, 318, 170, 50, 'ATA ×2: disk, CD', 'KeyLargoATA', 'open')
    p += box(385, 318, 140, 50, 'VIA', 'AppleVIA', 'open')
    p += box(540, 318, 185, 50, 'PMU: clock, power', 'ApplePMU', 'closed')
    p += box(35, 383, 170, 50, 'NVRAM', 'AppleCore99NVRAM', 'open')
    p += box(220, 383, 150, 50, 'GPIO', 'AppleGPIO', 'open')
    p += box(385, 383, 160, 50, 'Media bay', 'AppleMediaBay', 'open')
    p += legend(470, 760)
    return svg(760, 522, 'Block diagram of QEMU’s mac99 with the driver of each block', p)


def imac():
    p = []
    p.append('<text x="380" y="24" text-anchor="middle" font-size="15">iMac G5 (PowerMac12,1)</text>')
    p += box(280, 40, 200, 50, 'PowerPC 970FX', 'AppleMacRISC4PE', 'open')
    p += line(380, 90, 380, 115)
    p += container(20, 115, 720, 90, 'U4 north bridge: AppleMacRISC4PE')
    p += box(35, 140, 115, 50, 'Memory', '', 'none')
    p += box(160, 140, 130, 50, 'DART: IOMMU', 'AppleDART', 'open')
    p += box(300, 140, 120, 50, 'MPIC', 'AppleMPIC', 'open')
    p += box(430, 140, 140, 50, 'I²C: 4 sensors', 'IOI2CFamily', 'open')
    p += box(580, 140, 145, 50, 'PCI Express', 'AppleMacRiscPCI', 'open')
    p += line(652, 190, 652, 230)
    p += box(560, 230, 170, 50, 'ATI RV370 GPU', 'ATIRadeon9700', 'closed')
    p += line(380, 205, 380, 230)
    p += box(270, 230, 220, 50, 'HyperTransport', 'AppleMacRiscHT', 'open')
    p += line(380, 280, 380, 305)
    p += container(20, 305, 720, 160, 'K2 I/O controller and the PCI buses behind it (IOPCI2PCIBridge)')
    p += box(35, 333, 200, 50, 'SATA: internal disk', 'AppleK2SATARoot, AppleK2SATA', 'open')
    p += box(245, 333, 140, 50, 'ATA: optical drive', 'AppleKauaiATA', 'open')
    p += box(395, 333, 160, 50, 'GMAC Ethernet', 'AppleGMACEthernet', 'open')
    p += box(565, 333, 160, 50, 'USB: OHCI, EHCI', 'IOUSBFamily', 'open')
    p += box(35, 398, 255, 50, 'mac-io: MPIC, GPIO, I²C, I²S', 'AppleKeyLargo, AppleGPIO, AppleI2S', 'open')
    p += box(300, 398, 200, 50, 'Audio: PCM3052 codec', 'AppleOnboardAudio, OpenOnyx', 'ours')
    p += box(510, 398, 95, 50, 'FireWire', 'AppleFWOHCI', 'closed')
    p += box(615, 398, 110, 50, 'AirPort', 'AirPortBrcm43xx', 'closed')
    p += box(35, 490, 255, 50, 'SMU: clock, power, fans', 'OpenSMU', 'ours')
    p += box(300, 490, 200, 50, 'Boot ROM and NVRAM', 'AppleFlashNVRAM', 'open')
    p += box(510, 490, 215, 50, 'Bluetooth, on USB', 'IOBluetoothFamily', 'closed')
    p += legend(565, 760)
    return svg(760, 617, 'Block diagram of the iMac G5 with the driver of each block', p)


def disk():
    p = []
    p.append('<text x="380" y="24" text-anchor="middle" font-size="15">The test image: one disk with an Apple Partition Map</text>')
    p += box(20, 42, 170, 50, 'Block 0', 'driver descriptor map', 'none')
    p += box(195, 42, 210, 50, 'Partition 1', 'Apple_partition_map', 'none')
    p += box(410, 42, 330, 50, 'Partition 2: Darwin9', 'Apple_HFS', 'none')
    p += line(575, 92, 575, 120)
    p += container(20, 120, 720, 270, 'The Darwin9 volume, HFS+')
    p += box(35, 150, 250, 50, 'Volume header', 'Finder info word 0: blessed folder', 'none')
    rows = [
        ('/', ''),
        ('\u251c\u2500 mach_kernel', 'the kernel'),
        ('\u251c\u2500 System/Library/', ''),
        ('\u2502  \u251c\u2500 CoreServices/', 'the blessed folder'),
        ('\u2502  \u2502  \u251c\u2500 BootX', 'type tbxi, creator chrp'),
        ('\u2502  \u2502  \u2514\u2500 SystemVersion.plist', ''),
        ('\u2502  \u251c\u2500 Extensions/', 'the kexts'),
        ('\u2502  \u2514\u2500 Extensions.mkext', 'the driver cache'),
        ('\u2514\u2500 Library/Preferences/SystemConfiguration/', ''),
        ('   \u2514\u2500 com.apple.Boot.plist', 'kernel name and flags'),
    ]
    y = 160
    for path, note in rows:
        p.append(f'<text x="315" y="{y}" font-size="12.5" font-family="Menlo, Consolas, monospace" xml:space="preserve">{escape(path)}</text>')
        if note:
            p.append(f'<text x="725" y="{y}" font-size="12" text-anchor="end">{escape(note)}</text>')
        y += 22
    p.append(f'<line x1="285" y1="175" x2="310" y2="222" stroke="{STROKE}" stroke-width="1.5" stroke-dasharray="4 3"/>')
    return svg(760, 405, 'Layout of the test image: block 0, the partition map, and the Darwin9 HFS+ volume with its blessed folder, BootX, mach_kernel and the driver cache', p)


out = sys.argv[1]
open(f'{out}/powerpc-macs-mac99.svg', 'w').write(mac99())
open(f'{out}/powerpc-macs-imac-g5.svg', 'w').write(imac())
open(f'{out}/boot-chain-disk-layout.svg', 'w').write(disk())
