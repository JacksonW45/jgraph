CC      = g++
CFLAGS  = -std=c++17 -Wall -Wextra

SRC_DIR = src
BIN_DIR = bin
PDF_DIR = pdf

TARGET  = $(BIN_DIR)/jgraph

COURT   = $(SRC_DIR)/court.jgr

# 5 ESPN recap links (edit these to whatever you want)
GAME1 = https://www.espn.com/mens-college-basketball/recap/_/gameId/401808255
GAME2 = https://www.espn.com/mens-college-basketball/game/_/gameId/401808199
GAME3 = https://www.espn.com/mens-college-basketball/recap/_/gameId/401820771
GAME4 = https://www.espn.com/mens-college-basketball/game/_/gameId/401825543
GAME5 = https://www.espn.com/mens-college-basketball/game/_/gameId/401808271

all: dirs build pdfs

dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(PDF_DIR)

build: dirs
	$(CC) $(CFLAGS) $(SRC_DIR)/jgraph.cpp -o $(TARGET)

# Helper: render + pdf + (optional) crop
define RENDER_GAME
	@cp $(COURT) $(BIN_DIR)/court.jgr
	@cd $(BIN_DIR) && ./jgraph "$(2)"
	@jgraph $(BIN_DIR)/home.jgr > $(BIN_DIR)/$(1)_home.ps
	@jgraph $(BIN_DIR)/away.jgr > $(BIN_DIR)/$(1)_away.ps
	@ps2pdf $(BIN_DIR)/$(1)_home.ps $(PDF_DIR)/$(1)_home.pdf
	@ps2pdf $(BIN_DIR)/$(1)_away.ps $(PDF_DIR)/$(1)_away.pdf
	@command -v pdfcrop >/dev/null 2>&1 && pdfcrop $(PDF_DIR)/$(1)_home.pdf $(PDF_DIR)/$(1)_home.pdf >/dev/null || true
	@command -v pdfcrop >/dev/null 2>&1 && pdfcrop $(PDF_DIR)/$(1)_away.pdf $(PDF_DIR)/$(1)_away.pdf >/dev/null || true
endef

pdfs: build
	$(call RENDER_GAME,game1,$(GAME1))
	$(call RENDER_GAME,game2,$(GAME2))
	$(call RENDER_GAME,game3,$(GAME3))
	$(call RENDER_GAME,game4,$(GAME4))
	$(call RENDER_GAME,game5,$(GAME5))
	@echo ""
	@echo "Done. PDFs are in $(PDF_DIR)/"

clean:
	rm -rf $(BIN_DIR) $(PDF_DIR)

.PHONY: all dirs build pdfs clean