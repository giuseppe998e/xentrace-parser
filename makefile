CC = gcc
CFLAGS = -Os -s
CINCLD = -I $(LIBDIR)

CP = cp
RM = rm -f
MKD = mkdir

LIBDIR = ./lib
SRCDIR = ./src
OUTDIR = ./out

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(subst $(SRCDIR), $(OUTDIR), $(SOURCES:.c=.o))

#---
.PHONY: build
build: $(OBJECTS) $(OUTDIR)/xentrace-parser.h $(OUTDIR)/xentrace-event.h

# ---
$(OUTDIR)/%.o: $(SRCDIR)/%.c
	@$(MKD) -p $(dir $@)
	@$(CC) $(CFLAGS) $(CINCLD) -c $< -o $@

# ---
$(OUTDIR)/%.h: $(SRCDIR)/%.h
	@$(MKD) -p $(dir $@)
	@$(CP) $< $@

# ---
.PHONY: clean
clean:
	@$(RM) -r $(OUTDIR)