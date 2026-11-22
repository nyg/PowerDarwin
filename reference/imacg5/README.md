# iMac G5 under Leopard

What Leopard runs on the iMac G5 (`PowerMac12,1`), against which PowerDarwin's image is compared: the drivers it loads (`kextstat.txt`, `loaded-kexts.tsv`, `kextfind-loaded.txt`), the installed extensions (`extensions.txt`) and packages (`packages.txt`), the kernel's boot log (`boot-kernel-log.txt`, with MAC addresses masked after the vendor prefix), and the system version (`system.txt`).

They were captured under Sorbet Leopard, a community update of Leopard 10.5.8 whose kernel and drivers carry the versions of 10.5.8's build list, which is why `system.txt` reports `10.5.9` build `9L31b`. [hardware/powermac12-1.md](../../hardware/powermac12-1.md) notes the one exception, AppleGPIO.
