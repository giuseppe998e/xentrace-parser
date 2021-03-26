CC = gcc
CFLAGS = -Os -s
CINCLD = -I $(LIBDIR)

CP = cp
RM = rm -f
MKD = mkdir

LIBDIR = ./lib
SRCDIR = ./src
OUTDIR = ./out

#---
.PHONY: build
build: $(OUTDIR)/xentrace-parser.o $(OUTDIR)/xentrace-parser.h $(OUTDIR)/xentrace-event.h

# ---
$(OUTDIR)/%.o: $(SRCDIR)/%.c
	@$(MKD) -p $(OUTDIR)
	@$(CC) $(CFLAGS) $(CINCLD) -c $< -o $@

# ---
$(OUTDIR)/%.h: $(SRCDIR)/%.h
	@$(MKD) -p $(OUTDIR)
	@$(CP) $< $(OUTDIR)

# ---
.PHONY: clean
clean:
	@$(RM) -r $(OUTDIR)