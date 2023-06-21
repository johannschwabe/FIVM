# Usage: make [APP=(regression|regression_categorical|factorized_join|listing_join)] [DATASET=(retailer|housing|tpch|simple)]

# Compilers
FRONTEND_BIN = ../bin/run_frontend.sh
BACKEND_BIN = ../bin/run_backend.sh
CC = g++
CFLAGS = -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic -Wno-unused-local-typedefs

# Runtime arguments
NUMBER_OF_RUNS ?= 1
BATCH_SIZE ?= 1000
APP ?=

# Directories
SRC_DIR = src
SQL_DIR = queries
M3_OUT_DIR = generated/m3
CPP_OUT_DIR = generated/cpp
CONFIG_OUT_DIR = generated/config
APP_OUT_DIR = generated/application
EXE_OUT_DIR = bin
APP_GEN = generated/app_generator

# Find all SQL queries
SQL_FILES := $(shell find $(SQL_DIR) -name "*.sql")

# Filter queries by application
ifneq ($(APP), )
SQL_FILES := $(filter %_$(APP).sql, $(SQL_FILES))
endif

# Filter queries by dataset
ifneq ($(DATASET),)
SQL_FILES := $(filter $(wildcard */$(DATASET)/*.sql), $(SQL_FILES))
endif

# SQL to M3 compilation
M3_FILES_BATCH := $(patsubst $(SQL_DIR)/%.sql, $(M3_OUT_DIR)/%_BATCH.m3, $(SQL_FILES))
M3_FILES_SINGLE := $(patsubst $(SQL_DIR)/%.sql, $(M3_OUT_DIR)/%_SINGLE.m3, $(SQL_FILES))

# Config to application compilation
APPLICATION_FILES := $(patsubst $(SQL_DIR)/%.sql, $(APP_OUT_DIR)/%.hpp, $(SQL_FILES))

# M3 to CPP compilation
CPP_FILES_BATCH := $(patsubst $(M3_OUT_DIR)/%.m3, $(CPP_OUT_DIR)/%.hpp, $(M3_FILES_BATCH))
CPP_FILES_SINGLE := $(patsubst $(M3_OUT_DIR)/%.m3, $(CPP_OUT_DIR)/%.hpp, $(M3_FILES_SINGLE))

# CPP compilation
EXE_FILES_BATCH := $(patsubst $(CPP_OUT_DIR)/%.hpp, $(EXE_OUT_DIR)/%_$(BATCH_SIZE), $(CPP_FILES_BATCH))
EXE_FILES_SINGLE := $(patsubst $(CPP_OUT_DIR)/%.hpp, $(EXE_OUT_DIR)/%, $(CPP_FILES_SINGLE))

.PHONY: all m3_batch m3_single m3 cpp_batch cpp_single cpp exe_batch exe_single exe clean

all: exe

$(APP_GEN): $(SRC_DIR)/result_enumeration.cpp
	$(CC) $(CFLAGS) $^ -o $@

$(M3_FILES_BATCH): $(M3_OUT_DIR)/%_BATCH.m3: $(SQL_DIR)/%.sql
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@test -d $(dir $(patsubst generated/m3/%,generated/config/%,$@)) || mkdir -p $(dir $(patsubst generated/m3/%,generated/config/%,$@))
	$(FRONTEND_BIN) --batch -o $@ $<

$(M3_FILES_SINGLE): $(M3_OUT_DIR)/%_SINGLE.m3: $(SQL_DIR)/%.sql
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@test -d $(dir $(patsubst generated/m3/%,generated/config/%,$@)) || mkdir -p $(dir $(patsubst generated/m3/%,generated/config/%,$@))
	$(FRONTEND_BIN) --single -o $@ $<

$(APPLICATION_FILES): $(APP_OUT_DIR)/%.hpp: $(M3_OUT_DIR)/%_SINGLE.m3 $(APP_GEN)
	@test -d $(dir $@) || mkdir -p $(dir $@)
	./$(APP_GEN) $(patsubst %.hpp,%.txt,$(patsubst $(APP_OUT_DIR)/%,$(CONFIG_OUT_DIR)/%,$@)) $@

$(CPP_FILES_BATCH): $(CPP_OUT_DIR)/%.hpp : $(M3_OUT_DIR)/%.m3
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(BACKEND_BIN) --batch -o $@ $<

$(CPP_FILES_SINGLE): $(CPP_OUT_DIR)/%.hpp : $(M3_OUT_DIR)/%.m3
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(BACKEND_BIN) -o $@ $<

$(EXE_FILES_BATCH): $(EXE_OUT_DIR)/%_BATCH_$(BATCH_SIZE) : $(CPP_OUT_DIR)/%_BATCH.hpp $(SRC_DIR)/main.cpp $(APP_OUT_DIR)/%.hpp
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(eval DATASET := $(patsubst $(EXE_OUT_DIR)/%/, %, $(dir $@)))
	$(eval APP_HEADER := $(patsubst $(EXE_OUT_DIR)/$(DATASET)/%_BATCH_${BATCH_SIZE}, $(APP_OUT_DIR)/$(DATASET)/%.hpp, $@))
	$(eval CUSTOMHASHMAPARGS = $(shell zsh custom_hashmap.zsh $(DATASET)))
	$(CC) $(CFLAGS) $(CUSTOMHASHMAPARGS) $(SRC_DIR)/main.cpp -I ../backend/lib -I $(SRC_DIR) -I $(SRC_DIR)/lib -I $(SRC_DIR)/TLXInstall/include -L $(SRC_DIR)/TLXInstall/lib -ltlx -DBATCH_SIZE=$(BATCH_SIZE) -include $< -include $(APP_HEADER) -o $@; \


$(EXE_FILES_SINGLE): $(EXE_OUT_DIR)/%_SINGLE : $(CPP_OUT_DIR)/%_SINGLE.hpp $(SRC_DIR)/main.cpp $(APP_OUT_DIR)/%.hpp
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(eval DATASET := $(patsubst $(EXE_OUT_DIR)/%/,%, $(dir $@)))	
	$(eval APP_HEADER := $(patsubst $(EXE_OUT_DIR)/$(DATASET)/%_SINGLE, $(APP_OUT_DIR)/$(DATASET)/%.hpp, $@))
	$(eval CUSTOMHASHMAPARGS = $(shell zsh custom_hashmap.zsh $(DATASET)))
	$(CC) $(CFLAGS) $(CUSTOMHASHMAPARGS) $(SRC_DIR)/main.cpp -I ../backend/lib -I $(SRC_DIR) -I $(SRC_DIR)/lib -I $(SRC_DIR)/TLXInstall/include -L $(SRC_DIR)/TLXInstall/lib -ltlx -include $< -include $(APP_HEADER) -o $@; \


m3_batch: $(M3_FILES_BATCH)

m3_single: $(M3_FILES_SINGLE)

m3: m3_batch m3_single 

cpp_batch: $(CPP_FILES_BATCH) 

cpp_single: $(CPP_FILES_SINGLE)

cpp: cpp_batch cpp_single

exe_batch: $(EXE_FILES_BATCH) 

exe_single: $(EXE_FILES_SINGLE)

exe: exe_batch

clean: 
	rm -fr $(M3_OUT_DIR) $(CPP_OUT_DIR) $(EXE_OUT_DIR) $(CONFIG_OUT_DIR) $(APP_OUT_DIR) $(APP_GEN)
