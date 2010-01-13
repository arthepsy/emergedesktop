LIBS += -lemergeLib \
				-lcomctl32 \
				-lshlwapi \
				-lemergeBaseClasses

TARGET = reg2xml
SUFFIX = .exe
OBJS = $(TARGET_DIR)/main.o \
			 $(TARGET_DIR)/Importer.o \
			 $(TARGET_DIR)/CoreSettings.o \
			 $(TARGET_DIR)/AppletSettings.o \
			 $(TARGET_DIR)/CommandSettings.o \
			 $(TARGET_DIR)/LauncherSettings.o \
			 $(TARGET_DIR)/VWMSettings.o \
			 $(TARGET_DIR)/TasksSettings.o \
			 $(TARGET_DIR)/TraySettings.o \
			 $(TARGET_DIR)/HotkeysSettings.o \
			 $(TARGET_DIR)/DesktopSettings.o \
			 $(TARGET_DIR)/$(TARGET).res

include ../functions.in
include ../globals.in
include ../applet_globals.in
include ../hg_file.in
