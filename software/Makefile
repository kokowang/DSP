library:=lib
apps:=apps
export COMPILER:=$(if $(COMPILER),$(COMPILER),g++)
export CROSSCOMP:=$(if $(CROSSCOMP),$(CROSSCOMP),arm-linux-gnueabihf-g++)
export OBJ_DIR:=$(if $(OBJ_DIR),$(OBJ_DIR),$(PWD)/build)
export PROJECT:=$(if $(PROJECT),$(PROJECT),piecewise)

.PHONY: all $(library) $(apps) clean

all: $(library) $(apps)

$(library): 
	$(MAKE) --directory=$@ 

$(apps):$(library)
	$(MAKE) --directory=$@ 

clean:
	$(RM) -r build
