#pragma once

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wnarrowing"

#define DEFAULT_DISPLAY 0

#define MEMPAK_MAX_ENTRIES 16

#include <libdragon.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <vector>
#include <list>
#include <array>
#include <functional>
#include <map>
#include <stdio.h>
#include <iostream>

/**
 * @file LibN64.h
 * @brief C++ wrapper for Libdragon N64 ROM production. 
 * @details Includes a multitude of classes to aid in the production 
 * 			of your applications, a mixture of object and static classes.
 * @note Under development
 * @ingroup LibN64
 */

/*	CLASSES:

	LibN64::LibDFS 	(QuickRead<> -> static)
	LibN64::DMA 	(static)
		LibN64::IO  (static)
	LibN64::EEPROM  (static)
	LibN64::LibMemPak

	LibN64::Frame 
		LibN64::LibColor
		LibN64::LibPos
		LibN64::Lib2DVec<type>
		LibN64::Math (static)

	LibN64::LibMenu
	LibN64::LibMenuManager
	LibN64::LibSprite
	LibN64::LibRTC

	LibN64::Audio::WavAudio
*/

/*entirely contextual*/
using ID	= int;
using Byte 	= char;

namespace LibN64 
{
	class LibDFS 
	{
		private:
			uint32_t dfsHandle;

		public:
		LibDFS(){}

		/**
		 * @brief Construct a new LibDFS
		 * 		  Automatically open new file handle
		 * @param file 
		 */
		LibDFS(const std::string& file) { Open(file); }

		/**
		 * @brief Opens a DFS file
		 * @param  char* file 
		 * @return handle to the file  
		 */
		uint32_t Open(const std::string& file) 
		{
			dfsHandle = dfs_open(file.c_str());
			return dfsHandle;
		}
		
		/**
		 * @brief Grabs the currently open DFS's file size
		 * @return Current size of open file
		 */
		uint32_t Size()
		{
			return dfs_size(dfsHandle);
		}
		
		/**
		 * @brief Returns whether at EOF or not
		 * 
		 * @return int 
		 */
		uint32_t AtEOF() 
		{
			return dfs_eof(dfsHandle);
		}

		/**
		 * @brief Returns the ROM address of where the specified file is stored
		 * 
		 * @param file 
		 * @return uint32_t 
		 */
		uint32_t GetROMAddress(const std::string& file) 
		{
			return dfs_rom_addr(file.c_str());
		}

		 /** 
		 * @brief Closes the DFS session 
		 */
		void Close()
		{
			dfs_close(dfsHandle);
		}

		uint32_t GetHandle() 
		{
			return this->dfsHandle;
		}
		
		/** 
		 * @brief Opens a file for reading with a user-provided buffer
		 * @param T buffer
		 * @param uint32_t size
		 * @param uint32_t offset (to read from)
		 */
		template<class T>
		T Read(uint32_t size, uint32_t offset = 0x0) 
		{
			T buffer = static_cast<T>(std::malloc(size));
			dfs_seek(dfsHandle, offset, SEEK_SET);
			if(size > 0)
				dfs_read(buffer, 1, size, dfsHandle);

			return buffer;
		}
		
		/**
		 * @brief Opens a file, reads it to a buffer, and returns the buffer contents all in one go.
		 * Great for if you do not need to do anything specifical during the reading.
		 * 
		 * @tparam T 
		 * @param file file path 
		 * @return T specified buffer that will be returned (int, float, char, sprite_t* etc)
		 */
		template<class T>
		static T QuickRead(const std::string& file) 
		{
			int tmp = dfs_open(file.c_str());
			T buffer = static_cast<T>(std::malloc(dfs_size(tmp)));
			
			dfs_read(buffer, 1, dfs_size(tmp), tmp);
			dfs_close(tmp);
			
			return buffer;
		}

	};

	class DMA 
	{
		public:
			template<class T>
			/**
			 * @brief Read in a block of memory from PI to specified RAM buffer 
			 * 
			 * @param PI PI Address
			 * @param RAM RAM Address
			 * @param Length Size to read
			 */
			inline static void FromPI(const int PI, const T RAM, size_t Length) 
			{
				dma_read(reinterpret_cast<void*>(RAM), PI, Length);
			}

			/**
			 * @brief Write a block of memory from RAM to PI
			 * 
			 * @tparam T 
			 * @param RAM RAM Address
			 * @param PI PI Address
			 * @param Length Size to write
			 */
			template<class T>
			inline static void ToPI(const T RAM, const int PI, size_t Length) 
			{
				dma_write(reinterpret_cast<void*>(RAM), PI, Length);
			}

			inline static int Busy() {
				return dma_busy();
			}

			inline static void Wait() {
				while(dma_busy());
			}

		class IO 
		{
			public:
				inline static uint32_t Read(const auto address) 
				{
					return io_read(address);
				}

				inline static void Write(const auto address, uint32_t data) {
					io_write(address, data);
				}
		};

	};

	class EEPROM
	{
		public:
			enum Type { ENONE, E4K, E16K };

			/**
			 * @brief Get the current type of EEPROM available 
			 * 
			 * @return EEPROM Type
			 */
			inline static Type GetType() 
			{
				return static_cast<Type>(eeprom_present());
			} 

			/**
			 * @brief Get the Total Blocks available to write 
			 * 
			 * @return size_t Blocks available
			 */
			inline static size_t GetTotalBlocks() {
				return eeprom_total_blocks();
			}

			/**
			 * @brief Read 8 bytes of data from 1 block on the EEPROM
			 * 
			 * @tparam T 
			 * @param block 
			 * @return T Block of data in data storage of users choosing 
			 */
			template<class T>
			inline static T Read(uint8_t block) 
			{
				uint8_t *tmp = new uint8_t[EEPROM_BLOCK_SIZE];
				eeprom_read(block, tmp);

				return reinterpret_cast<T>(tmp);
			}

			/**
			 * @brief Read a specified amount of data throughout several blocks 
			 * 		  on the eeprom
			 * 
			 * @tparam T 
			 * @param offset 
			 * @param length 
			 * @return T Specified amounts of data in storage of users choosing
			 */
			template<class T>
			inline static T ReadBytes(uint32_t offset, size_t length = sizeof(T)) 
			{
				uint8_t *tmp = new uint8_t[length];
				eeprom_read_bytes(tmp, offset, length);

				return reinterpret_cast<T>(tmp);
			}

			/**
			 * @brief Write a block of data to the EEPROM
			 * 
			 * @tparam T Can be a struct, class, data structure of any kind
			 * @param block 
			 * @param data 
			 */
			template<class T>
			inline static void Write(uint8_t block, T *data) 
			{
				eeprom_write(block, data);
			}

			/**
			 * @brief Write a specified amount of bytes to the EEPROM
			 * 
			 * @tparam T 
			 * @param data data to write
			 * @param offset offset to begin writing to
			 * @param size size that you would like to write (default parameter set)
			 */
			template<class T>
			inline static void WriteBytes(T data, uint32_t offset = 0x0, size_t size = sizeof(T)) 
			{
				eeprom_write_bytes(reinterpret_cast<uint8_t*>(data), offset, size);
			}
	};

	/**
	 * @brief The main MemPak interface 
	 * 
	 */
	class LibMemPak
	{
	private:
		int pakControllerID;
		int	pakValidEntries;
		int pakBlocksFree;
		uint8_t *pakEntryData;

	public:
		enum Pads 
		{
			CONTROLLER_1,
			CONTROLLER_2,
			CONTROLLER_3,
			CONTROLLER_4
		};

	private:
		std::vector<entry_structure_t> 	pakEntries;
		std::string pakFileName;

		/**
		 * @brief This reads in the entries of the pak to the vector 
		 */
		void _ReadPakEntries()
		{
			if(!pakEntries.empty())
				pakEntries.empty();
			for(auto spot = 0; spot < MEMPAK_MAX_ENTRIES; spot++) 
			{
				entry_structure_t tmpEntry;
				get_mempak_entry(pakControllerID, spot, &tmpEntry);
				pakEntries.push_back(tmpEntry);
				
				if(tmpEntry.valid) {
					pakValidEntries++;
				}
			}
			pakBlocksFree = get_mempak_free_space(pakControllerID);
		}

	public:
	/**
	 * @brief Constructs a new LibMemPak object
	 * 	
	 * @param entryfilename Entry filename that will be used when writing (e.g. "MEMPAK.Z")
	 * @param controller 0-4
	 */
		LibMemPak(std::string entryfilename, int controller) : pakControllerID(controller), pakFileName(entryfilename)
		{
			_ReadPakEntries();
		}

		/**
		 * @brief Reads a Pak entry supplied by the pak entry structure into local buffer
		 * 		  and returns that buffer to the user
		 * @tparam RT 
		 * @param entryID Entry ID
		 * @return Any supplied datatype array
		 */
		template<class RT>
		RT ReadMemPakEntry(ID entryID)
		 {
			if(MemPakInserted() && IsValid()) 
			{
				entry_structure_t tmp;
				get_mempak_entry(pakControllerID, entryID, &tmp);

				if(tmp.valid) 
				{
					pakEntryData = new uint8_t[tmp.blocks * MEMPAK_BLOCK_SIZE];
					read_mempak_entry_data(pakControllerID, &tmp, pakEntryData);
					
					return reinterpret_cast<RT>(pakEntryData);
				}
			}
			return nullptr;
		}

		/**
		 * @brief Deletes a Pak entry specified by the ID
		 * 
		 * @param entryID 
		 */
		void DeleteMemPakEntry(ID entryID)
		{
			if(pakEntries.at(entryID).valid) 
				delete_mempak_entry(pakControllerID, &pakEntries[entryID]);

			_ReadPakEntries();
		}

		/**
		 * @brief Writes a Pak entry by ID only if the entry is not valid
		 * 
		 * @tparam DataArray 
		 * @param entryID Entry ID
		 * @param pakdata Data to write
		 */
		template<typename DataArray>
		void WriteMemPakEntry(ID entryID, const DataArray pakdata) 
		{
			entry_structure_t entry = pakEntries.at(entryID);

			if(!entry.valid) 
			{
				strcpy(entry.name, pakFileName.c_str());
				entry.blocks = 1;
				entry.region = 0x45;
				write_mempak_entry_data(pakControllerID, &entry, reinterpret_cast<uint8_t*>(pakdata));

				_ReadPakEntries();
			}
		}

		/**
		 * @brief Writes to the first open and available space that is not valid
		 * 
		 * @tparam DataArray 
		 * @param pakdata Data to write
		 */
		template<typename DataArray>
		void WriteAnyMemPakEntry(const DataArray pakdata) 
		{
			for(auto& entry : pakEntries) 
			{
				if(!entry.valid) 
				{
					strcpy(entry.name, pakFileName.c_str());
					entry.blocks = 1;
					entry.region = 0x45;
					write_mempak_entry_data(pakControllerID, &entry, reinterpret_cast<uint8_t*>(pakdata));
					break;
				}
			}

			_ReadPakEntries();
		}

		/**
		 * @brief Looks up an entry by it's name and will return the first entry it comes across
		 * @param entryname Prospective entry name (e.g. "MEMPAK.Z")
		 * @return Entry ID
		 */
		int FindFirstEntryWith(const std::string entryname) {
			for(int spot = 0; spot < pakEntries.size(); spot++) 
			{
				if(strcmp(pakEntries.at(spot).name, entryname.c_str()) == 0) 
				{
					return spot;
				}
			} 
		}

		/**
		 * @brief Get the Entry Structure by supplying a known ID
		 * @param entryID Entry ID
		 * @return entry_structure_t 
		 */
		entry_structure_t GetEntryStructure(ID entryID) 
		{
			return pakEntries.at(entryID);
		} 


		/**
		 * @brief Gets the Pak entry name by supplying a known ID
		 * @param entryID Entry ID
		 * @return std::string 
		 */
		std::string GetMemPakEntryName(ID entryID) {
			return pakEntries.at(entryID).name;
		}

		/**
		 * @brief Formats the MemPak
		 */
		void FormatMemPak() 
		{
			format_mempak(pakControllerID);
		}

		/**
		 * @brief Enumerates all valid entries contained 
		 * @return int 
		 */
		int GetValidEntries() 
		{
			return pakValidEntries;
		}

		/**
		 * @brief Get all available blocks free that are left to be written
		 * @return Number of blocks as an INT
		 */
		int GetBlocksFree() 
		{
			return pakBlocksFree;
		}

		/**
		 * @brief Get the File Handle object
		 * @return std::string 
		 */
		std::string GetFileHandle()
		{
			return pakFileName;
		}

		/**
		 * @brief Returns whether the Pak is inserted or not
		 * @return true 
		 * @return false 
		 */
		bool MemPakInserted() 
		{
			controller_data cTmp;
			get_accessories_present(&cTmp);

			if(identify_accessory(pakControllerID) == ACCESSORY_MEMPAK) 
			{
				return true;
			}
			return false;
		}

		/**
		 * @brief Returns whether the Pak is valid 
		 * @return true 
		 * @return false 
		 */
		bool IsValid() 
		{
			if(MemPakInserted()) 
			{
				if(validate_mempak(pakControllerID) == 0)
				 {
					return true;
				}
				return false;
			}
		}
		
	};
};

namespace LibN64 
{	
	/**
	* @brief Converts seperate R,G,B,A and mixes down into one color 

	* @param  int Red  
	* @param  int Green
	* @param  int Blue
	* @param  int Alpha
	* @return int 32bit Color
	*/
	constexpr int MakeColor(int r, int g, int b, int a) 
	{
		return (r << 24) | (((g & 0x00FFFFFF) << 16)) | (((b & 0xFF00FFFF) << 8)) | ((a & 0xFFFF00FF));
	}

	/**
	 * @brief Predefined colors to choose from when writing colors to a shape or object
	 * 
	 */
	enum LibColor 
	{
		 RED			= MakeColor(0xFF, 0x00, 0x00, 0xFF),
		 GREEN			= MakeColor(0x00, 0xFF, 0x00, 0xFF),
		 WHITE			= MakeColor(0xFF, 0xFF, 0xFF, 0xFF),
		 BLACK			= MakeColor(0x00, 0x00, 0x00, 0x00),
		 BLUE			= MakeColor(0x00, 0x00, 0xFF, 0xFF),
		 NAVY_BLUE		= MakeColor(0x11, 0x18, 0x80, 0xFF),
		 DARK_RED		= MakeColor(0x4B, 0x13, 0x00, 0xFF),
		 DEEP_DARK_RED	= MakeColor(0x1F, 0x01, 0x00, 0xFF),
		 SKY_BLUE		= MakeColor(0x11, 0x18, 0xD7, 0xFF),
		 ORANGE			= MakeColor(0xFF, 0xA5, 0x00, 0xFF),
		 GOLD			= MakeColor(0xFF, 0xD7, 0x00, 0xFF),
		 YELLOW			= MakeColor(0xFF, 0xFF, 0x00, 0xFF),
		 CYAN			= MakeColor(0x00, 0xFF, 0xFF, 0xFF),
		 GREY			= MakeColor(0x80, 0x80, 0x80, 0xFF),
		 PURPLE			= MakeColor(0xFF, 0x00, 0x9B, 0xFF)
	};

	/**
	 * @brief Simple supply struct for objects that require X and Y coordinate spaces
	 */
	struct LibPos 
	{ 
		int x, y; 
		LibPos operator +(LibPos x) const 
		{
			LibPos tmp = { this->x, this->y };
			tmp.x += x.x;
			tmp.y += x.y;
			return tmp;
		}

		LibPos operator -(LibPos x) const 
		{
			LibPos tmp = { this->x, this->x};
			tmp.x -= x.x;
			tmp.y -= x.y;
			return tmp;
		}
				
		bool operator ==(LibPos x) const			
		{
			LibPos tmp = { this->x, this->y };
			return (tmp.x == x.x && tmp.y == x.y) ? true : false;
		}
	};

	/**
	 * @brief Simple 2D open-type supply struct for any object that requires it
	 * @tparam SpecifiedType 
	 */
	template<class SpecifiedType>
	struct Lib2DVec
	{
		public:
		std::pair<SpecifiedType, SpecifiedType> values;

		SpecifiedType& First() {
			return values.first;
		}

		SpecifiedType& Second() {
			return values.second;
		}
	};

	/**
	 * @brief Main core of the library
	 * @details Inside, the main loop is included along with many of the core
	 * 			functions required to get a Libdragon game up and running.
	 */
	class Frame 
	{
		private:
			display_context_t 	d;
			resolution_t 		r;
			bitdepth_t 			bd;
			antialias_t			aa;
			sprite_t* 		  libFont = nullptr;

			int 	screenWidth;
			int 	screenHeight;
			
			bool 	bActive;
			bool    bDLInLoop = false;

			
			float 	fFrameTime; 
			float	fTotalTime;
			float   fFrameRate;
			float   fFrameCounter;
			int     iFrames;

			int 	uitype;

			/*Local RDP function*/
			inline void rdp_quickattach() 
			{
				rdp_init();
				rdp_set_default_clipping();
				rdp_enable_blend_fill();
				rdp_attach_display(GetDisplay());
				rdp_enable_primitive_fill();
			}

			/*Local RDP Function*/
			inline void rdp_quickdetach()
			{
				rdp_sync(SYNC_PIPE);
				rdp_detach_display();
				rdp_close();
			}
			
		public:
			virtual void FrameUpdate();
			virtual void OnCreate();
			virtual void KeyAPressed();
			virtual void KeyBPressed();
			virtual void KeyZPressed();

			virtual void KeyStartPressed();

			virtual void KeyDUPressed();
			virtual void KeyDDPressed();
			virtual void KeyDLPressed();
			virtual void KeyDRPressed();

			virtual void KeyCUPressed();
			virtual void KeyCDPressed();
			virtual void KeyCLPressed();
			virtual void KeyCRPressed();
			
			virtual void KeyJoyXPressed(int);
			virtual void KeyJoyYPressed(int);

			inline virtual void     __OnInit_FreeFunction1();
			inline virtual void     __OnInit_FreeFunction2();
			inline virtual void     __OnLoop_FreeFunction1();
			inline virtual void     __OnLoop_FreeFunction2();

		public:
			enum     UIType        { GUI, CONSOLE };
			enum     Joystick      { JoyUp=0x00000072, JoyDown=0x0000008E, JoyLeft=0x00008E00, JoyRight=0x00007200 };
			enum 	 KeyState 	   { KeysHeld, KeysDown, KeysPressed, KeysUp } kstate;
			
			/**
			 * @brief Does the dirty work of setting the resolution values
			 * @param  resolution_t resolution
			 */
			void CheckAndSwitchRes(resolution_t r) 
			{
				auto sr = [&](int x, int y)  
				{
					this->screenWidth = x;
					this->screenHeight = y;
				};

				switch (r) 
				{
					case RESOLUTION_320x240: sr(320, 240); break;
					case RESOLUTION_256x240: sr(256, 240); break;
					case RESOLUTION_640x240: sr(640, 240); break;
					case RESOLUTION_640x480: sr(640, 480); break;
					case RESOLUTION_512x240: sr(512, 240); break;
					case RESOLUTION_512x480: sr(512, 480); break;
					default: sr(320, 240); break;
				}
			}
			
			/*Empty frame constructor*/
			Frame() {}
			
			/**
			 * @brief Sets up the entire frame with values required to render LibN64 functions
			 *
			 * @param	resolution_t Resolution type
			 * @param	bitdepth_t   Bitdepth
			 * @param	antialias_t   Anti-aliasing
			 * @param	UIType 		  UI Type (Console or GUI)
			 */
			Frame(resolution_t res, bitdepth_t dep, antialias_t aa, UIType ui) 
			: r(res), bd(dep), aa(aa), uitype(ui)
			{
				d 		= DEFAULT_DISPLAY;
				kstate 	= KeysHeld;
				CheckAndSwitchRes(res);
			
				controller_init();
				dfs_init(DFS_DEFAULT_LOCATION);

				switch(ui)
				{
					case GUI:
					{
						display_init(res, dep, 3, GAMMA_NONE, aa);

						while(!(d = display_lock()));

						audio_init(44100, 4);
						mixer_init(16);  
						mixer_ch_set_limits(2, 0, 128000, 0);

						graphics_fill_screen(d, 0x0);
						graphics_set_color(0xFFFFFFFF, 0x0);
					} break;
					case CONSOLE: 
					{
						console_set_render_mode(RENDER_AUTOMATIC);
						console_init();
						console_render();
					} break; 
					default: __assert(__FILE__,__LINE__, "Invalid UI selection (Must be either GUI or Console)"); break;
				}
				bActive = true;
			}

	
			/**
			 * @brief This is called as soon as the frame has been initialized.
			 * 		  The loop is established here.
			 */
			void Begin() 
			{
				if(!bDLInLoop) 
					display_show(d);

				this->OnCreate();

				this->__OnInit_FreeFunction1();
				this->__OnInit_FreeFunction2();
			
				while (bActive) 
				{
					if(bDLInLoop)
						d = display_lock();	

					timer_init();
					float fTmp = timer_ticks();
					this->__OnLoop_FreeFunction1();
								
					this->FrameUpdate();

						controller_scan();
						controller_data keys;
						switch(kstate)
						{
							case KeysHeld:    keys = get_keys_held(); break;
							case KeysDown:    keys = get_keys_down(); break;
							case KeysPressed: keys = get_keys_pressed(); break;
							case KeysUp:      keys = get_keys_up(); break;
							default: break;
						};
						
						if (keys.c[0].err == ERROR_NONE) 
						{
							int data = keys.c[0].data;
						
							if (keys.c[0].A)       { this->KeyAPressed();    }
							if (keys.c[0].B)       { this->KeyBPressed();    }
							if (keys.c[0].up)      { this->KeyDUPressed();   }
							if (keys.c[0].down)    { this->KeyDDPressed();   }
							if (keys.c[0].left)    { this->KeyDLPressed();   }
							if (keys.c[0].right)   { this->KeyDRPressed();   }
							if (keys.c[0].Z)       { this->KeyZPressed();    }
							if (keys.c[0].start)   { this->KeyStartPressed();}
							if (keys.c[0].C_up)    { this->KeyCUPressed();   }
							if (keys.c[0].C_down)  { this->KeyCDPressed();   }
							if (keys.c[0].C_left)  { this->KeyCLPressed();   }
							if (keys.c[0].C_right) { this->KeyCRPressed();   }
							if (keys.c[0].x)	   { this->KeyJoyXPressed(data & 0x0000FF00); } 
							if (keys.c[0].y) 	   { this->KeyJoyYPressed(data & 0x000000FF); }
						}
					
						if(uitype == CONSOLE) 
							console_render();
						
						fFrameTime = timer_ticks();

					#ifndef LIBN_BUILD_CONSOLE
						fFrameTime -= fTmp;
						fFrameCounter += fFrameTime; 
						fTotalTime += fFrameTime;
					#endif

						timer_close();	

						if(bDLInLoop) display_show(d);
						    
						this->__OnLoop_FreeFunction2();
				}
			}

			
			/**
			 * @brief Stop the loop and continue execution in the main()
			 */
			void Close() 
			{
				 bActive = false;
			}

			/** @brief Clears the screen with a specified color, selective to what type of GUI mode is enabled.
			 * 			If no color is specified, it is defaulted to black.
			 * @param  uint32_t color
			 */
			void ClearScreen(uint32_t color = BLACK)
			{
				switch(uitype) 
				{
					case GUI    : graphics_fill_screen(d, color);
					case CONSOLE: console_clear();
				}
			}

			/** @brief Clears the screen with a specified color but with RDP (Hardware)
			 *  @param  uint32_t color 
			 */
			void ClearScreenRDP(uint32_t color = BLACK) 
			{
				DrawRectRDP({0,0},{ScreenWidth(),ScreenHeight()}, color);
			}
			
			/** @brief Changes screen resolution on the fly
			 *  @param  resolution_t Resolution
			 *  @param  bitdepth_t Bitdepth
			 */
			void SetScreen(resolution_t res, bitdepth_t bd)
			{
				CheckAndSwitchRes(res);

				display_close();
				display_init(res, bd, 3, GAMMA_NONE, this->aa);
				d = display_lock();
			}

			/** @brief Returns the frames display context
			 * @return display_context_t Handle to LibN64::Frame display
			 */
			display_context_t GetDisplay() { return this->d;}

			/**
			 * @brief Returns the current Anti-Aliasing Mode
			 * 
			 * @return antialias_t Antialiasing
			 */
			antialias_t GetAAMode() { return this->aa; }

			/**
			 * @brief Returns the current bitdepth
			 * 
			 * @return bitdepth_t Bitdepth
			 */
			bitdepth_t GetBitdepth() { return this->bd; }

			/**
			 * @brief Returns the current resolution
			 * 
			 * @return resolution_t Resolution
			 */
			resolution_t GetResolution() { return this->r; }
			
			/**
			 * @brief Sets the key state in the loop (whether it's keydown, pressed, up, or held).
			 * @param  KeyState Frame::KeyState 
			 */
			void SetKeyState(KeyState k) { kstate = k;}

			/**
			 * @brief Sets the display_lock inside the loop which is ideal for most applications but some 
			 * such as my CirclesAndSquares demo flicker and refresh oddly with it
			 */
			void SetDLInLoop() { bDLInLoop = true;}
			
			/**
			 * @brief Returns screenwidth 
			 * @return int Width
			 */
			int ScreenWidth() { return screenWidth;}

			/**
			 * @brief Returns screenheight 
			 * @return int Height
			 */
			int ScreenHeight() { return screenHeight;};

			/** @brief DrawText feature with ability to use as printf
			 * 
			 * @param  LibPos Position on screen         
			 * @param  std::string Format
			 * @param  ... Variadic arguments     
			 */
			void DrawTextFormat(LibPos pos, const std::string format, ...) 
			{
				va_list args;
				va_start(args, format.c_str());

				char *buffer = new char[300];
				vsprintf(buffer, format.c_str(), args);
				DrawText(pos, buffer);

				va_end(args);	
				
				delete buffer;
			}

			/** @brief DrawText feature with ability to use as printf with coloring
			 * 
			 * @param  LibPos Position on screen 
			 * @param  std::string Format 
			 * @param  ... Variadic arguments     
			 */
			void DrawTextFormat(LibPos pos, LibColor forecolor, LibColor backcolor, const std::string format, ...)
			{
				va_list args;
				va_start(args, format.c_str());

				char *buffer = new char[300];
				vsprintf(buffer, format.c_str(), args);
				DrawText(pos, buffer, forecolor, backcolor);

				va_end(args);	

				delete buffer;
			}
	
			/**
			 * @brief Draw's 8x8 text to the screen at the specified location
			 * @param  LibPos Position
			 * @param  std::string Text    
			 * @param  LibColor Forecolor 
			 * @param  LibColor Backcolor 
			 */
			void DrawText(LibN64::LibPos pos, const std::string t, LibColor forecolor = WHITE, LibColor backcolor = BLACK) 
			{
				if(forecolor != WHITE || backcolor != 0x0) 
				{
					graphics_set_color(forecolor, backcolor);
					graphics_draw_text(this->d, pos.x, pos.y, t.c_str());
					graphics_set_color(WHITE, 0);
				} 
				else
					graphics_draw_text(this->d, pos.x, pos.y, t.c_str());
			}

			/** 
			 * @brief Draws pixel to the screen at the specified location
			 * @param  LibPos Position 
			 * @param  uint32_t Color
			 */
	 		void DrawPixel(LibPos pos, uint32_t color = WHITE) 
			{
				graphics_draw_pixel(this->d, pos.x, pos.y, color);
			}
			
			/** @brief Can draw either a filled or wireframe rectangle at the specified position with 
			 *		   specified dimensions and color
			 * 
			 * @param  LibPos Position           
			 * @param  LibPos Dimensions
			 * @param  uint32_t Color          
			 * @param  bool True or False
			 */

			void DrawRect(LibPos pos, LibPos dimensions={1,1}, const uint32_t color = WHITE, bool isFilled = true) 
			{
				if(isFilled) 
				{
					graphics_draw_box(d, pos.x, pos.y, dimensions.x, dimensions.y, color);
				} 
				else {
					DrawLine(pos, {pos.x + dimensions.x, pos.y}, color); DrawLine({pos.x + dimensions.x, pos.y}, {pos.x+dimensions.x, pos.y + dimensions.y}, color);
					DrawLine({pos.x+dimensions.x, pos.y+dimensions.y}, {pos.x, pos.y + dimensions.y}, color); 
					DrawLine({pos.x, pos.y + dimensions.y}, pos, color);
				}
			}

			/**
			 * @brief Draws a filled rectangle at the specified position with alpha support
			 * @param Pos 
			 * @param Dimensions 
			 * @param Color 
			 */
			void DrawRectTrans(LibPos pos, LibPos dimensions={1,1}, const uint32_t color = WHITE) 
			{
				graphics_draw_box_trans(d, pos.x, pos.y, dimensions.x, dimensions.y, color);
			}

	
			/**
			* @brief  Initializes RDP, draws an RDP Rectangle to the screen then closes RDP.
			* @param  LibPos Position           
			* @param  LibPos Dimensions         
			* @param  uint32_t Color          
			*/
			void DrawRectRDP(LibPos pos, LibPos dimensions={1,1}, const uint32_t color = WHITE) 
			{
				rdp_quickattach();
				rdp_set_primitive_color(color);
				rdp_set_blend_color(color);
				rdp_draw_filled_rectangle(pos.x,pos.y,dimensions.x,dimensions.y);
				rdp_quickdetach();
			}

			/**
			 * @brief Draws a line to the screen
			 * 
			 * @param pos1 Start X and Y
			 * @param pos2  End X and Y
			 * @param color Specified color of the line 
			 */
	 		void DrawLine(LibPos pos1, LibPos pos2, const uint32_t color = WHITE)
			{
				graphics_draw_line(this->d, pos1.x, pos1.y, pos2.x, pos2.y, color);
			}

			/**
			 * @brief Draws a circle to the screen with customizable step size and optional filling and scale
			 * 
			 * @param pos Position
			 * @param scale Scale
			 * @param cStepSize The rate at which the angle is increased
			 * @param color Color
			 * @param isFilled Circle is filled or not
			 */
			void DrawCircle(LibPos pos, int scale = 1, const uint32_t color = WHITE, bool isFilled = true, float cStepSize = 0.1) 
			{
				if(isFilled) 
				{
					for(float scaler = 0;scaler<=scale;scaler+=0.3) 
					{
						for(float angles =0;angles<25*scaler;angles+=cStepSize)
						{ 
							DrawPixel({pos.x + cosf(angles) * 3.1415f * scaler, pos.y + sinf(angles) * 3.1415f * scaler}, color);
						}
					}
				} 
				else 
				{
					for(float angles =0;angles<25*scale;angles+=cStepSize) 
					{
						DrawPixel({pos.x + cosf(angles) * 3.1415f * scale, pos.y + sinf(angles) * 3.1415f * scale}, color);
					}
				}
			}

			/**
			 * @brief Draws a software triangle to the screen at the specified location
			 * @param pos1 Point 1
			 * @param pos2 Point 2
			 * @param pos3 Point 3
			 * @param color  Specified color
			 */
			void DrawTri(LibPos pos1,LibPos pos2,LibPos pos3, uint32_t color = WHITE) 
			{
				DrawLine({pos1.x,pos1.y},{pos2.x,pos2.y}, color);
				DrawLine({pos2.x,pos2.y},{pos3.x,pos3.y}, color);
				DrawLine({pos3.x,pos3.y},{pos1.x,pos1.y}, color);
			}

			/**
			 * @brief Draws an RDP (Hardware) triangle to the screen at the specified locations
			 * @param pos1 Point 1
			 * @param pos2 Point 2
			 * @param pos3 Point 3
			 * @param color Specified fill color
			 */
			void DrawTriRDP(LibPos pos1,LibPos pos2,LibPos pos3, uint32_t color = WHITE) 
			{
				rdp_quickattach();
				rdp_set_blend_color(color);
				rdp_draw_filled_triangle(pos1.x,pos1.y,pos2.x,pos2.y,pos3.x,pos3.y);
				rdp_quickdetach();
			}

			/**
			 * @brief Draws a sprite (sprite_t*) at the specified location
			 * @param pos Position
			 * @param spr Sprite handle
			 */
			void DrawSprite(LibPos pos, sprite_t* spr) {
				graphics_draw_sprite(this->d, pos.x, pos.y, spr);
			}

			/**
			 * @brief Draws a sprite from a sprite-map 
			 * 
			 * @param pos Position
			 * @param offset Map offset
			 * @param spr Sprite handle
			 */
			void DrawSpriteStride(LibPos pos, uint32_t offset, sprite_t* spr) {
				graphics_draw_sprite_stride(this->d, pos.x, pos.y, spr, offset);
			}

			/**
			 * @brief Draws a sprite with alpha
			 * 
			 * @param pos Position
			 * @param spr Sprite handle
			 */
			void DrawSpriteTrans(LibPos pos, sprite_t* spr) {
				graphics_draw_sprite_trans(this->d, pos.x, pos.y, spr);
			}

			/**
			 * @brief Draws a sprite from a sprite-map with alpha 
			 * 
			 * @param pos Position
			 * @param offset Map offset
			 * @param spr Sprite handle
			 */
			void DrawSpriteTransStride(LibPos pos, uint32_t offset, sprite_t* spr) {
				graphics_draw_sprite_trans_stride(this->d, pos.x, pos.y, spr, offset);
			}

			/**
			 * @brief Converts ticks to seconds for easy readability 
			 * @param float Ticks 
			 * @return float Amount of seconds in ticks 
			 */
			float Ticks2Seconds(float t) 
			{
				return (t * 0.021333333 / 1000000.0);
			}

			/**
			 * @brief Use's "baremetal" MIPS 8x8 font (Peterlemon from GitHub)
			 * @details Load your own font and draw with the DrawTextCF and DrawText
			 *			FormatCF functions 
			 * @param FileName Path to the font file
			 */
			void LoadCustomFont(const std::string FileName) 
			{
				libFont = LibDFS::QuickRead<sprite_t*>(FileName.c_str());
				if(libFont == nullptr) {
					assertf(libFont != nullptr, "There was an error loading the custom font.");
				}
			}

			/**
			 * @brief Draw text from the custom font file that has been loaded with LoadCustomFont
			 * 
			 * @param pos Position
			 * @param str Text
			 * @param LibColor Color (optional)
			 */
			void DrawTextCF(LibPos pos, const std::string str, int LibColor = WHITE) 
			{
				uint32_t incx = pos.x;
				uint32_t incy = pos.y;
				for(size_t index = 0;index<str.length();index++) 
				{
					if(incx >= ScreenWidth()) { incy+=8; incx = pos.x; }

					graphics_draw_font(GetDisplay(), incx, incy, LibColor, libFont, str[index]);
					incx += 8;
				}
			}

			/**
			 * @brief Draw text from the custom font file, but with printf-formatting 
			 * 
			 * @param pos Position
			 * @param format Format
			 * @param ... Variadic Arguments
			 */
			void DrawTextFormatCF(LibPos pos, const std::string format, ...) 
			{
				va_list args;
				va_start(args, format.c_str());

				char buffer[300];
				vsprintf(buffer, format.c_str(), args);	

				DrawTextCF(pos, buffer);

				va_end(args);	
			}

			/**
			 * @brief Get the Total Time elapsed since start of the application
			 * 
			 * @return float Total time elapsed
			 */
			float GetTotalTime() 
			{ 
				return Ticks2Seconds(fTotalTime);
			}

			/**
			 * @brief Get the time it takes to draw one frame 
			 * 
			 * @return float Frame time
			 */
			float GetFrameTime() {

				return Ticks2Seconds(fFrameTime);
			};

			/**
			 * @brief Get how many frames are drawn per second
			 * 
			 * @return float Frame rate
			 */
			float GetFrameRate() 
			{ 
				return (1 / GetFrameTime());
			};

			/*The following functions refuse to compile inside the C++ file.*/
			/*DFS does not work so here is work around. Manually find*/
		public:
			/**
			 * @brief Convert ROM or RAM to an array of type T (struct, char* array, etc)
			 * 
			 * @tparam T 
			 * @param romAddr ROM (0xB0000000) or RAM address
			 * @param size Size of memory you'd like to collect
			 * @return T* 
			 */
			template<class T>
			T* __libn_rom2buf(long romAddr, int size) {
				T* tmp = static_cast<T*>(std::malloc(size + sizeof(T)));
				for (auto i = 0; i < size; i++) {
					tmp[i] = __lib64_rom2type<T>(romAddr + (i*sizeof(T)));
				}
				return tmp;
			}

			/*Helper function for above*/
			template<class T>
			T __libn_rom2type(long romAddr) {
				T *ptr = static_cast<T*>(romAddr);
				return *ptr;
			}

		/**
		 * @brief Simple math helper.
		 * @note Very small, still under construction.
		 */
		class LibMath
		{
		public:
			constexpr static double PI = 3.1415926f;

			/**
			 * @brief Determine if an object is inside a circle with a known radius 
			 * 
			 * @param obj1 Object
			 * @param obj2 Circle-object
			 * @param cradius Radius of Circle-object
			 * @return true or false 
			 */
			static bool IsPointInsideCircle(LibPos obj1, LibPos obj2, float cradius)
			{
				return sqrt((obj1.x-obj2.x)*(obj1.x-obj2.x) + (obj1.y-obj2.y)*(obj2.y-obj2.y)) < cradius;
			}

			/**
			 * @brief Simple Pythagorean Theorem to determine distance 
			 * 
			 * @param obj1 LibPos of obj1 
			 * @param obj2 LibPos of obj2
			 * @return int Distance
			 */
			static int CalculateDistance(LibPos obj1, LibPos obj2) 
			{
				return sqrt(((obj2.x - obj1.x)*(obj2.x - obj1.x)) + ((obj2.y - obj1.y)*(obj2.y-obj1.y)));
			}
		};
	};

	/**
	 * @brief Rudimentary menu system. Seems to work fine on emulators but has trouble
	 * 		  on hardware
	 * @bug Not working on hardware
	 * 
	 */
	class LibMenu 
	{
		private:
			ID					mId;
			LibN64::LibPos 		mPos;
			std::string 		mTitle;
			std::string 		mContent;
			LibN64::LibColor	mForecolor, mBackcolor;

			int			mMenuItemSpacing = 10;
			int			mMenuItemCount;
			float		mMenuItemSelection;

			std::map<int,std::string>			mMenuItems;
			std::vector<std::function<void()>> 	mMenuItemCallbacks;
			std::array<bool, 27>				mMenuItemsSelected;

			bool bMenuIsShowing   = true;
			bool bEnableHighlight = true;
			bool bKeyStateHeld    = true;
			LibColor cHighlightColor = LibColor::RED;
		public:
			bool 		bInFocus;		
		public:
			LibMenu(ID i, std::string t, LibPos p, LibColor f = BLACK, LibColor b = WHITE)
			{ 
				mId = i; mPos = p; mTitle = t; mForecolor = f; mBackcolor = b;
			}

			template<class FunctionCallback>
			void AddMenuItem(int mId, std::string content, FunctionCallback call) 
			{
				mMenuItems[mId] = content;
				mMenuItemCallbacks.push_back(call);
				mMenuItemCount++;
			}

			void AddMenuItem(int mId, std::string content) 
			{
				mMenuItems[mId] = content;
				mMenuItemCallbacks.push_back([this]{});
				mMenuItemCount++;
			}


			void MoveSelectionUp(Frame &r)   
			{ 
				mMenuItemsSelected.fill(false);

				if((mMenuItemSelection - 1) >= 0 
				&& bInFocus && MenuIsShowing()) 
					mMenuItemSelection-= (r.kstate == Frame::KeyState::KeysHeld || r.kstate == Frame::KeyState::KeysPressed) ? 0.02 : 1; 
			}

			void MoveSelectionDown(Frame &r) 
			{ 
				mMenuItemsSelected.fill(false);

				if((mMenuItemSelection + 1) < mMenuItemCount
				 && bInFocus && MenuIsShowing()) 
					mMenuItemSelection+= (r.kstate == Frame::KeyState::KeysHeld || r.kstate == Frame::KeyState::KeysPressed) ? 0.02 : 1; 
			}

			void Show(LibN64::Frame& lFrame)
			{
				lFrame.SetKeyState(Frame::KeyState::KeysDown);

				if(bMenuIsShowing) 
				{
					auto findLargestMenuItem = [&]() -> int
					{
						std::vector<int> stringLengths;
						for(auto& t : mMenuItems) {
							stringLengths.push_back(t.second.length());
						}
						stringLengths.push_back(this->mTitle.length());

						std::sort(stringLengths.begin(), stringLengths.end(), [](int a, int b) {
							return a > b;
						});

						return stringLengths.front();
					};
					
					auto toUpper = [=](std::string str) -> std::string
					{
						std::string tmp;
						for(auto& character : str) {
							tmp += toupper(character);
						}
						return tmp;
					};

					LibPos dimensions = 
					{ 
						10 * findLargestMenuItem(), 
						35 + (10 * mMenuItemCount)
					};
					
					
					lFrame.DrawRect(mPos, dimensions, mForecolor);
					lFrame.DrawRect(mPos, dimensions, mBackcolor, false);
		
					lFrame.DrawRect(mPos, {dimensions.x, 15}, mBackcolor);
					lFrame.DrawText({mPos.x+5, mPos.y+5},  mTitle, mForecolor, mBackcolor);

					int incy = 20, spot = 0;
					for(auto &t : mMenuItems) 
					{
						if(bInFocus && (int)mMenuItemSelection == spot) 
						{
							if(bEnableHighlight) 
							{
								lFrame.DrawRect({mPos.x+2, mPos.y+incy-2}, {dimensions.x-3, 10}, cHighlightColor);
							}

							lFrame.DrawText({mPos.x+5, mPos.y+incy}, toUpper(t.second));
						} 
						else 
							lFrame.DrawText({mPos.x+5, mPos.y+incy}, t.second);
						incy+=mMenuItemSpacing;
						++spot;
					}
				}
			}

			bool MenuItemIsSelected(int item) 
			{
				return mMenuItemsSelected[item];
			}

			bool MenuIsShowing() {
				return bMenuIsShowing;
			}

			void Hide() {
		
				bMenuIsShowing = false;
				SetUnfocused();
			}

			void EnableShow() {
				bMenuIsShowing = true;
			}

			void EnableHighlight() {
				bEnableHighlight = true;
			}

			void DisableHighlight() {
				bEnableHighlight = false;
			}

			void SetHighlightColor(LibColor c) {
				cHighlightColor = c;
			}

			void WaitKeyPress() {
				if(bInFocus)
				{
					mMenuItemsSelected.at(mMenuItemSelection) = true;
					mMenuItemCallbacks.at(mMenuItemSelection)();
				}
			}
			void SetItemSpacing(int spacing) {
				mMenuItemSpacing = spacing;
			}

			void SetFocused() {
				bInFocus = true;
			}

			void SetUnfocused() {
				bInFocus = false;
			}
	};

	class LibMenuManager
	{
		private:
			std::vector     <LibMenu*>  menuList;
			std::map    <ID, LibMenu*>  menuMap;

		public:
			void AddMenu(ID i, std::string t, LibPos p, LibColor f, LibColor b)
			{
				/*menuList.push_back(new LibMenu(i, t, p, f, b));
				menuMap[i] = menuList.back();*/

				auto *tmp = new LibMenu(i, t, p, f, b);
				menuList.push_back(tmp);
				menuMap[i] = tmp;
			}

			bool AllMenusClosed() 
			{
				for(auto &menus : menuList) {
					if(menus->MenuIsShowing() || menus->bInFocus) {
						return false;
					}
				}
				return true;
			}

			void CloseFocusedMenus() 
			{
				for(auto& menus : menuList) {
					if(menus->bInFocus) {
						menus->Hide();
					}
				}
			}

			void CloseAllMenus() 
			{
				for(auto& menus : menuList) {
					menus->Hide();
				}
			}

			LibMenu* operator [](ID i) { return menuMap[i]; }
	};

	/** 
	 * @brief Optional class wrapper for sprite
	 * */
	class LibSprite 
	{
		private:
			sprite_t *sSpr = nullptr;
		public:
			LibSprite(const std::string& fp) { this->Load(fp); }
			LibSprite() {}

			void Load(const std::string& filePath) 
			{
				sSpr = LibDFS::QuickRead<sprite_t*>(filePath.c_str());
			}

			void Draw(LibN64::Frame &r, LibPos pos) 
			{
				r.DrawSprite(pos, this->sSpr);
			}

			void DrawFromMap(LibN64::Frame &r, LibPos pos, uint8_t offset) 
			{
				r.DrawSpriteStride(pos, offset, this->sSpr);
			}

			void Delete() {
				delete sSpr;
			}

			sprite_t* Get() {
				return sSpr;
			}
	};

	/**
	 * @brief Real-Time-Clock class interface
	 */
	class LibRTC 
	{
		private:
			rtc_time_t rTimer;

			std::map<uint8_t, const std::string> bToMonth = 
			{
				{0,  "January"},
				{1,  "Feburary"},
				{2,  "March"},
				{3,  "April"},
				{4,  "May"},
				{5,  "June"},
				{6,  "July"},
				{7,  "August"},
				{8,  "September"},
				{9,  "October"},
				{10, "November"},
				{11, "December"},
			};

			std::map<uint8_t, const std::string> bToWDay = 
			{
				{0, "Sunday"},
				{1, "Monday"},
				{2, "Tuesday"},
				{3, "Wednesday"},
				{4, "Thursday"},
				{5, "Friday"},
				{6, "Saturday"}
			};

		public:
			inline void UpdateTime() {
				rtc_init();
				rtc_get(&this->rTimer);
			}
			
			uint8_t GetSeconds() {
				return rTimer.sec;
			}

			uint8_t GetMinutes() {
				return rTimer.min;
			}

			uint8_t GetHours() {
				return rTimer.hour % 12;
			}

			uint8_t GetDay() {
				return rTimer.day;
			}

			std::string GetMonth() {
				return bToMonth[rTimer.month];
			}

			std::string GetWeekday() {
				return bToWDay[rTimer.week_day];
			}
			uint16_t GetYear() {
				return rTimer.year;
			}
	};

	namespace Audio 
	{
		
		/**
		 * @brief Unsure of the status of this. My build cant find the symbols
		 * 		  for Wav64_open and others.
		 * 
		 */
		class WavAudio
		{
			public:
				wav64_t wTrack;
				int 	iLocalChannel;
				WavAudio(){}
				WavAudio(std::string sTrackTitle) 
				{
					Init(sTrackTitle);
				}

				void Init(std::string sTrackTitle) 
				{
					wav64_open(&wTrack, sTrackTitle.c_str());
					iLocalChannel = 0;
				}

				void Play()
				{
					wav64_play(&wTrack, 0);
				}

				void SetVolume() 
				{
					mixer_ch_set_vol(iLocalChannel, 0.25f, 0.25f);
				}
		};
	};
}

/* initialize the functions for override as initializing in class seems to be broken*/
namespace LibN64 
{
	void LibN64::Frame::FrameUpdate()  {}
	void LibN64::Frame::KeyAPressed()  {}
	void LibN64::Frame::KeyStartPressed(){}
	void LibN64::Frame::KeyBPressed()  {}
	void LibN64::Frame::KeyDUPressed() {}
	void LibN64::Frame::KeyDDPressed() {}
	void LibN64::Frame::KeyDLPressed() {}
	void LibN64::Frame::KeyDRPressed() {}
	void LibN64::Frame::KeyCUPressed() {}
	void LibN64::Frame::KeyCDPressed() {}
	void LibN64::Frame::KeyCLPressed() {}
	void LibN64::Frame::KeyCRPressed() {}
	void LibN64::Frame::KeyZPressed()  {}
	void LibN64::Frame::KeyJoyXPressed(int){}
	void LibN64::Frame::KeyJoyYPressed(int){}
	void LibN64::Frame::OnCreate()     {}
	inline void LibN64::Frame::__OnInit_FreeFunction1() {}
	inline void LibN64::Frame::__OnInit_FreeFunction2() {}
	inline void LibN64::Frame::__OnLoop_FreeFunction1() {}
	inline void LibN64::Frame::__OnLoop_FreeFunction2() {}
}


