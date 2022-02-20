.PHONY: all
all: tap dot

.PHONY: tap
tap:
	$(MAKE) -C tap

.PHONY: dot
dot:
	$(MAKE) -C dot

clean:
	rm -rf specasm specasm.zip
	- $(MAKE) -C tap clean
	- $(MAKE) -C dot clean

release:
	- rm -rf specasm
	mkdir specasm
	cp tap/*.tap specasm
	cp dot/SAIMPORT dot/SAEXPORT dot/*.X specasm
	cp ../bas/* specasm
	zip -r specasm.zip specasm
