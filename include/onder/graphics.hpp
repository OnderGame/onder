#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <PixelToaster.h>
#include <onder/collections/array.hpp>
#include <onder/collections/slice.hpp>
#include <onder/collections/list.hpp>

namespace onder {
namespace graphics {

using Pixel = PixelToaster::TrueColorPixel;

class Image {
	uint16_t m_width, m_height;
	Pixel *m_data;

	const Pixel *row(uint16_t y) const {
		return m_data + (m_width * y);
	}

	Pixel *row(uint16_t y) {
		return m_data + (m_width * y);
	}

	Image(const Image &) = delete;
	Image &operator=(const Image &) = delete;

public:
	Image() : m_data(nullptr), m_width(0), m_height(0) {}
	Image(uint16_t width, uint16_t height) : m_data(nullptr), m_width(width), m_height(height) {
		size_t len = (size_t)width * height;
		if (height != 0 && len / height != width)
			throw new std::exception(); // TODO specific error
		m_data = new Pixel[len];
	}
	Image(Pixel *data, uint16_t width, uint16_t height) : m_data(data), m_width(width), m_height(height) {}
	~Image() {
		delete m_data;
	}

	Image(Image &&src) : m_data(src.m_data), m_width(src.m_width), m_height(src.m_height) {
		src.m_width = 0;
		src.m_height = 0;
		src.m_data = nullptr;
	}
	Image &operator=(Image &&src) {
		m_width = src.m_width;
		m_height = src.m_height;
		m_data = src.m_data;
		src.m_width = 0;
		src.m_height = 0;
		src.m_data = nullptr;
		return *this;
	}

	static Image filled(uint16_t width, uint16_t height, Pixel value);
	static Image from_png(collections::Slice<const uint8_t> data);

	uint16_t width() const {
		return m_width;
	}

	uint16_t height() const {
		return m_height;
	}

	const Pixel *data() const {
		return m_data;
	}

	void copy_from(uint16_t x, uint16_t y, const Image &src);

	void fill(Pixel value) {
		for (size_t i = 0; i < (size_t)width() * height(); i++)
			m_data[i] = value;
	}
};

class Window {
	PixelToaster::Display display;
	Image buffer;

	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

public:
	Window(const char *title, uint16_t width, uint16_t height)
		: display(title, width, height, PixelToaster::Output::Default, PixelToaster::Mode::TrueColor)
		, buffer(width, height)
	{}

	bool is_open() const {
		return display.open();
	}

	void update() {
		display.update((Pixel *)buffer.data());
	}

	void draw(uint16_t x, uint16_t y, const Image &img);
	void clear(Pixel value);

	void set_listener(class InputListener &listener) {
		// These "visibility" rules are totally retarded.
		display.listener((PixelToaster::Listener *)&listener);
	}
};

struct Key {
	enum Code {
#define f(x) x = PixelToaster::Key::Code::x
		f(A), f(B), f(C), f(D), f(E),
		f(F), f(G), f(H), f(I), f(J),
		f(K), f(L), f(M), f(N), f(O),
		f(P), f(Q), f(R), f(S), f(T),
		f(U), f(V), f(W), f(X), f(Y), f(Z),
#undef f
		MOUSE_LEFT = 0x100,
		MOUSE_MIDDLE = 0x101,
		MOUSE_RIGHT = 0x102,
	} code;
};

enum class InputState {
	DOWN = 1,
	UP = 2,
	PRESSED = 3,
};

struct InputKey {
	Key key;
	InputState state;
};

class InputListener : public PixelToaster::Listener {
	/// Inputs since the last poll.
	collections::List<InputKey> m_events;
	/// Bitmask
	collections::Array<uint8_t, (256 + 8) / 8> m_keys_down;

public:
	InputListener() {}
	InputListener(const InputListener &) = delete;
	InputListener &operator=(const InputListener &) = delete;

	collections::Slice<const InputKey> events() const {
		return m_events;
	}

	void clear_events() {
		m_events.clear();
	}

    virtual bool defaultKeyHandlers() const { return true; }

    virtual void onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		PixelToaster::Key::Code code = key; // TODO how does this work? operator Code()
		m_events.push({ { (Key::Code)code }, InputState::DOWN });
	}

    virtual void onKeyPressed(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		PixelToaster::Key::Code code = key;
		m_events.push({ { (Key::Code)code }, InputState::PRESSED });
	}

    virtual void onKeyUp(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		PixelToaster::Key::Code code = key;
		m_events.push({ { (Key::Code)code }, InputState::UP });
	}

    virtual void onMouseButtonDown(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) override {
		//inputs.push({ { (Key::Code)key.Code() }, InputState::DOWN });
	}

    virtual void onMouseButtonUp(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) override {
		//inputs.push({ { (Key::Code)key.Code() }, InputState::UP });
	}

    virtual void onMouseMove(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) {}

    virtual void onActivate(PixelToaster::DisplayInterface& display, bool active) {}

    virtual void onOpen(PixelToaster::DisplayInterface& display) {}

    virtual bool onClose(PixelToaster::DisplayInterface& display) { return true; }
};

class Render {
public:
};

}
}
