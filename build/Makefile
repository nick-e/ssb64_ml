CORE_DIR = ../src/core
CORE_OBJS = \
	gamepad.o \
	gamepad_file.o \
	gamepad_listener.o \
	gamepad_spoofer.o \
	record_session.o \
	test_session.o \
	threadsafe_gamepad.o \
	timer.o \
	train_session.o \
	util.o \
	video_file.o \
	video_input.o \
	video_output.o

GUI_DIR = ../src/gui
GUI_OBJS = \
	app.o \
	gamepad_selector.o \
	gamepad_selector_popup.o \
	record_page.o \
	record_window.o \
	test_page.o \
	test_window.o \
	train_page.o \
	train_window.o \
	window_selector.o \
	window_selector_popup.o

LIBS = \
	-L/opt/cuda/lib64 \
	-lcuda \
	-lcudart \
	-lpthread \
	-lX11 \
	-lavformat \
	-lavcodec \
	-lavutil \
	-lxcb \
	`pkg-config --libs gtkmm-3.0`

INCL_DIRS = -I/opt/cuda/include
LFLAGS = -Wall -g
CFLAGS = -Wall -c -g
GTKMM_FLAGS = `pkg-config --cflags gtkmm-3.0`
NV_CFLAGS = -c -g
CC = clang++

all: $(CORE_OBJS) $(GUI_OBJS)
	$(CC) $(LFLAGS) -o ssbml $(CORE_OBJS) $(GUI_OBJS) $(LIBS)

gamepad.o: $(CORE_DIR)/gamepad.cc $(CORE_DIR)/gamepad.h
	$(CC) $(CFLAGS) $(CORE_DIR)/gamepad.cc $(INCL_DIRS)

gamepad_file.o: $(CORE_DIR)/gamepad_file.cc $(CORE_DIR)/gamepad_file.h
	$(CC) $(CFLAGS) $(CORE_DIR)/gamepad_file.cc $(INCL_DIRS)

gamepad_listener.o: $(CORE_DIR)/gamepad_listener.cc $(CORE_DIR)/gamepad_listener.h
	$(CC) $(CFLAGS) $(CORE_DIR)/gamepad_listener.cc $(INCL_DIRS)

gamepad_spoofer.o: $(CORE_DIR)/gamepad_spoofer.cc $(CORE_DIR)/gamepad_spoofer.h
	$(CC) $(CFLAGS) $(CORE_DIR)/gamepad_spoofer.cc $(INCL_DIRS)

record_session.o: $(CORE_DIR)/record_session.cc $(CORE_DIR)/record_session.h
	$(CC) $(CFLAGS) $(CORE_DIR)/record_session.cc $(GTKMM_FLAGS) $(INCL_DIRS)

test_session.o: $(CORE_DIR)/test_session.cc $(CORE_DIR)/test_session.h
	$(CC) $(CFLAGS) $(CORE_DIR)/test_session.cc $(GTKMM_FLAGS) $(INCL_DIRS)

threadsafe_gamepad.o: $(CORE_DIR)/threadsafe_gamepad.cc $(CORE_DIR)/threadsafe_gamepad.h
	$(CC) $(CFLAGS) $(CORE_DIR)/threadsafe_gamepad.cc $(INCL_DIRS)

timer.o: $(CORE_DIR)/timer.cc $(CORE_DIR)/timer.h
	$(CC) $(CFLAGS) $(CORE_DIR)/timer.cc $(INCL_DIRS)

train_session.o: $(CORE_DIR)/train_session.cc $(CORE_DIR)/train_session.h
	$(CC) $(CFLAGS) $(CORE_DIR)/train_session.cc $(GTKMM_FLAGS) $(INCL_DIRS)

util.o: $(CORE_DIR)/util.cc $(CORE_DIR)/util.h
	$(CC) $(CFLAGS) $(CORE_DIR)/util.cc $(INCL_DIRS)

video_file.o: $(CORE_DIR)/video_file.cu $(CORE_DIR)/video_file.h
	nvcc $(NV_CFLAGS) $(CORE_DIR)/video_file.cu $(INCL_DIRS)

video_input.o: $(CORE_DIR)/video_input.cu $(CORE_DIR)/video_input.h
	nvcc $(NV_CFLAGS) $(CORE_DIR)/video_input.cu $(INCL_DIRS)

video_output.o: $(CORE_DIR)/video_output.cc $(CORE_DIR)/video_output.h
	$(CC) $(CFLAGS) $(CORE_DIR)/video_output.cc $(INCL_DIRS)

app.o: $(GUI_DIR)/app.cc $(GUI_DIR)/app.h
	$(CC) $(CFLAGS) $(GUI_DIR)/app.cc $(GTKMM_FLAGS) $(INCL_DIRS)

gamepad_selector.o: $(GUI_DIR)/gamepad_selector.cc $(GUI_DIR)/gamepad_selector.h
	$(CC) $(CFLAGS) $(GUI_DIR)/gamepad_selector.cc $(GTKMM_FLAGS) $(INCL_DIRS)

gamepad_selector_popup.o: $(GUI_DIR)/gamepad_selector_popup.cc $(GUI_DIR)/gamepad_selector_popup.h
	$(CC) $(CFLAGS) $(GUI_DIR)/gamepad_selector_popup.cc $(GTKMM_FLAGS) $(INCL_DIRS)

record_page.o: $(GUI_DIR)/record_page.cc $(GUI_DIR)/record_page.h $(GUI_DIR)/app.h
	$(CC) $(CFLAGS) $(GUI_DIR)/record_page.cc $(GTKMM_FLAGS) $(INCL_DIRS)

record_window.o: $(GUI_DIR)/record_window.cc $(GUI_DIR)/record_window.h
	$(CC) $(CFLAGS) $(GUI_DIR)/record_window.cc $(GTKMM_FLAGS) $(INCL_DIRS)

test_page.o: $(GUI_DIR)/test_page.cc $(GUI_DIR)/test_page.h $(GUI_DIR)/app.h
	$(CC) $(CFLAGS) $(GUI_DIR)/test_page.cc $(GTKMM_FLAGS) $(INCL_DIRS)

test_window.o: $(GUI_DIR)/test_window.cc $(GUI_DIR)/test_window.h
	$(CC) $(CFLAGS) $(GUI_DIR)/test_window.cc $(GTKMM_FLAGS) $(INCL_DIRS)

train_page.o: $(GUI_DIR)/train_page.cc $(GUI_DIR)/train_page.h
	$(CC) $(CFLAGS) $(GUI_DIR)/train_page.cc $(GTKMM_FLAGS) $(INCL_DIRS)

train_window.o: $(GUI_DIR)/train_window.cc $(GUI_DIR)/train_window.h
	$(CC) $(CFLAGS) $(GUI_DIR)/train_window.cc $(GTKMM_FLAGS) $(INCL_DIRS)

window_selector.o: $(GUI_DIR)/window_selector.cc $(GUI_DIR)/window_selector.h
	$(CC) $(CFLAGS) $(GUI_DIR)/window_selector.cc $(GTKMM_FLAGS) $(INCL_DIRS)

window_selector_popup.o: $(GUI_DIR)/window_selector_popup.cc $(GUI_DIR)/window_selector_popup.h
	$(CC) $(CFLAGS) $(GUI_DIR)/window_selector_popup.cc $(GTKMM_FLAGS) $(INCL_DIRS)

clean:
	$(RM) $(CORE_OBJS) $(GUI_OBJS)
