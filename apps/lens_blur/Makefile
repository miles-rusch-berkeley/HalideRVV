include ../support/Makefile.inc


.PHONY: build clean test

build: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/process-riscv,\
											 $(BIN)/$(HL_TARGET)/process)
test: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/process-riscv.out,\
											$(BIN)/$(HL_TARGET)/out.png)
# dump-riscv: $(BIN)/$(HL_TARGET)/process-riscv.dump

$(BIN)/%/process-riscv: process.cpp $(BIN)/%/lens_blur.a $(BIN)/%/lens_blur_auto_schedule.a
	@mkdir -p $(@D)
	$(RISCV_GXX) $(RISCV_GXX_OPTS) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS)

$(BIN)/%/process-riscv.out: $(BIN)/%/process-riscv
	@mkdir -p $(@D)
	@echo "objump: $(RISCV_OBJDUMP) $< > $<.S"
	$(RISCV_OBJDUMP) $< > $<.S
	$(RISCV_SIM) $< 65 32 13 0.5 32 3 > $@
	

$(GENERATOR_BIN)/lens_blur.generator: lens_blur_generator.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(filter %.cpp,$^) -o $@ $(LIBHALIDE_LDFLAGS)

$(BIN)/%/lens_blur.a: $(GENERATOR_BIN)/lens_blur.generator
	@mkdir -p $(@D)
	$^ -g lens_blur -e $(GENERATOR_OUTPUTS) -o $(@D) -f lens_blur target=$*

$(BIN)/%/lens_blur_auto_schedule.a: $(GENERATOR_BIN)/lens_blur.generator
	@mkdir -p $(@D)
	$^ -g lens_blur -e $(GENERATOR_OUTPUTS) -o $(@D) -f lens_blur_auto_schedule target=$*-no_runtime autoscheduler=Mullapudi2016

$(BIN)/%/process: process.cpp $(BIN)/%/lens_blur.a $(BIN)/%/lens_blur_auto_schedule.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS)

$(BIN)/%/out.png: $(BIN)/%/process
	@mkdir -p $(@D)
	$< $(IMAGES)/rgb_small.png 32 13 0.5 32 3 $@

clean:
	rm -rf $(BIN)