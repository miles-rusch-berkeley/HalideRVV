include ../support/Makefile.inc


.PHONY: build dump-riscv clean test

build: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/sconv-run,\
											 $(BIN)/$(HL_TARGET)/process)
test: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/sconv-run.out,\
											$(BIN)/$(HL_TARGET)/out.png)
dump-riscv: $(BIN)/$(HL_TARGET)/sconv-run.S
trace-riscv: $(BIN)/$(HL_TARGET)/sconv-run.trace

$(BIN)/%/sconv-run: process.cpp $(BIN)/%/sconv_rvv.a $(BIN)/%/sconv_rvv_auto_schedule.a
	@mkdir -p $(@D)
	$(RISCV_GXX) $(RISCV_GXX_OPTS) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS)

$(BIN)/%/sconv-run.out: $(BIN)/%/sconv-run
	@mkdir -p $(@D)
	$(RISCV_SIM) $(PK) $< 33 > $@
	
$(BIN)/%/sconv-run.S: $(BIN)/%/sconv-run
	@mkdir -p $(@D)
	$(RISCV_OBJDUMP) $< > $@
	
$(BIN)/%/sconv-run.trace: $(BIN)/%/sconv-run
	@mkdir -p $(@D)
	$(RISCV_SIM) -l --log=$@ $(PK) $< 33
	
$(GENERATOR_BIN)/sconv_rvv.generator: sconv_rvv_generator.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(filter %.cpp,$^) -o $@ $(LIBHALIDE_LDFLAGS)

$(BIN)/%/sconv_rvv.a: $(GENERATOR_BIN)/sconv_rvv.generator
	@mkdir -p $(@D)
	$^ -g sconv_rvv -e $(GENERATOR_OUTPUTS) -o $(@D) -f sconv_rvv target=$*

$(BIN)/%/sconv_rvv_auto_schedule.a: $(GENERATOR_BIN)/sconv_rvv.generator
	@mkdir -p $(@D)
	$^ -g sconv_rvv -e $(GENERATOR_OUTPUTS) -o $(@D) -f sconv_rvv_auto_schedule target=$*-no_runtime autoscheduler=Mullapudi2016

$(BIN)/%/process: process.cpp $(BIN)/%/sconv_rvv.a $(BIN)/%/sconv_rvv_auto_schedule.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS)

$(BIN)/%/out.png: $(BIN)/%/process
	@mkdir -p $(@D)
	$< $(IMAGES)/rgb_small.png 32 13 0.5 32 3 $@

clean:
	rm -rf $(BIN)