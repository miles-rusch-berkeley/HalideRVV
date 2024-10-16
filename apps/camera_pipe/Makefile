include ../support/Makefile.inc

.PHONY: build dump-riscv clean test

build: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/process-riscv,\
											 $(BIN)/$(HL_TARGET)/process)
test: $(if $(findstring riscv,$(HL_TARGET)),$(BIN)/$(HL_TARGET)/out-riscv.print,\
											$(BIN)/$(HL_TARGET)/out.png)
dump-riscv: $(BIN)/$(HL_TARGET)/process-riscv.dump

### RISCV BUILD RULES ###
$(BIN)/%/process-riscv: process.cpp $(BIN)/%/camera_pipe.a $(BIN)/%/camera_pipe_auto_schedule.a
	@mkdir -p $(@D)
	$(RISCV_GXX) $(RISCV_GXX_OPTS) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS)

$(BIN)/%/out-riscv.print: $(BIN)/%/process-riscv
	@mkdir -p $(@D)
	$(RISCV_SIM) $< 2.0 50 1.0 $(TIMING_ITERATIONS) > $@
	cat $@

$(BIN)/%/process-riscv.dump: $(BIN)/%/process-riscv
	$(RISCV_OBJDUMP) $< > $@

### HOST BUILD RULES ###
TIMING_ITERATIONS ?= 5

$(GENERATOR_BIN)/camera_pipe.generator: camera_pipe_generator.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(filter-out %.h,$^) -o $@ $(LIBHALIDE_LDFLAGS) $(HALIDE_SYSTEM_LIBS)

$(BIN)/%/camera_pipe.a: $(GENERATOR_BIN)/camera_pipe.generator
	@mkdir -p $(@D)
	$^ -g camera_pipe -e $(GENERATOR_OUTPUTS) -o $(@D) -f camera_pipe target=$*

$(BIN)/%/camera_pipe_auto_schedule.a: $(GENERATOR_BIN)/camera_pipe.generator
	@mkdir -p $(@D)
	$^ -g camera_pipe -e $(GENERATOR_OUTPUTS) -o $(@D) -f camera_pipe_auto_schedule target=$*-no_runtime autoscheduler=Mullapudi2016

$(BIN)/%/process: process.cpp $(BIN)/%/camera_pipe.a $(BIN)/%/camera_pipe_auto_schedule.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Wall -I$(BIN)/$* $^ -o $@ $(IMAGE_IO_FLAGS) $(LDFLAGS)

$(BIN)/%/process_viz: process.cpp $(BIN)/%-trace_all/camera_pipe.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -DNO_AUTO_SCHEDULE -Wall -I$(BIN)/$*-trace_all $^ -o $@ $(IMAGE_IO_FLAGS) $(LDFLAGS)

$(BIN)/%/out.png: $(BIN)/%/process
	@mkdir -p $(@D)
	$(BIN)/$*/process $(IMAGES)/bayer_raw.png 3700 2.0 50 1.0 $(TIMING_ITERATIONS) $@ $(BIN)/$*/h_auto.png

../../bin/HalideTraceViz: ../../util/HalideTraceViz.cpp
	$(MAKE) -C ../../ bin/HalideTraceViz

$(BIN)/%/camera_pipe.mp4: $(BIN)/%/process_viz viz.sh $(HALIDE_TRACE_VIZ) ../../bin/HalideTraceViz
	HL_AVCONV=$(HL_AVCONV) bash viz.sh $(@D)

clean:
	rm -rf $(BIN)

viz: $(BIN)/$(HL_TARGET)/camera_pipe.mp4
	$(HL_VIDEOPLAYER) $^

$(BIN)/%/viz_auto.mp4: $(BIN)/%/process_viz ../support/viz_auto.sh ../../bin/HalideTraceViz
	@mkdir -p $(@D)
	HL_AVCONV=$(HL_AVCONV) bash ../support/viz_auto.sh "$(BIN)/$*/process_viz $(IMAGES)/bayer_small.png 3700 1.8 50 1 1 $(BIN)/$*/out.png" ../../bin/HalideTraceViz $@

viz_auto: $(BIN)/$(HL_TARGET)/viz_auto.mp4
	$(HL_VIDEOPLAYER) $^
