#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <onder/collections/array.hpp>
#include <onder/collections/slice.hpp>
#include <onder/collections/list.hpp>
#include <onder/math/vec2.hpp>
#include <onder/math/rect.hpp>

#pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# pragma GCC diagnostic ignored "-Wpedantic" // "ISO C++ prohibits anonymous structs"
# include <PixelToaster.h>
#pragma GCC diagnostic pop

namespace onder {
namespace graphics {

using Pixel = PixelToaster::TrueColorPixel;

class Image {
	Pixel *m_data;
	math::Vec2<uint16_t> m_dim;

	const Pixel *row(uint16_t y) const {
		return m_data + (m_dim.x * y);
	}

	Pixel *row(uint16_t y) {
		return m_data + (m_dim.x * y);
	}

	Image(const Image &) = delete;
	Image &operator=(const Image &) = delete;

public:
	Image() : m_data(nullptr) {}
	Image(math::Vec2<uint16_t> dim) : m_data(nullptr), m_dim(dim) {
		m_data = new Pixel[area()];
	}
	Image(Pixel *data, math::Vec2<uint16_t> dim) : m_data(data), m_dim(dim) {}
	~Image() {
		delete[] m_data;
	}

	Image(Image &&src) : m_data(src.m_data), m_dim(src.m_dim) {
		src.m_dim = {};
		src.m_data = nullptr;
	}
	Image &operator=(Image &&src) {
		this->~Image();
		m_dim = src.m_dim;
		m_data = src.m_data;
		src.m_dim = {};
		src.m_data = nullptr;
		return *this;
	}

	static Image filled(math::Vec2<uint16_t> dim, Pixel value);
	static Image from_png(collections::Slice<const uint8_t> data);

	math::Vec2<uint16_t> dim() const;
	uint32_t area() const;

	const Pixel *data() const {
		return m_data;
	}

	void copy_from(const Image &src, math::Rect<uint16_t> from, math::Vec2<uint16_t> to);

	void fill(Pixel value) {
		for (size_t i = 0; i < area(); i++)
			m_data[i] = value;
	}
};

class Window {
	PixelToaster::Display display;
	Image buffer;

	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

public:
	Window(const char *title, math::Vec2<uint16_t> dim)
		: display(title, dim.x, dim.y, PixelToaster::Output::Default, PixelToaster::Mode::TrueColor)
		, buffer(dim)
	{}

	bool is_open() const {
		return display.open();
	}

	void update() {
		display.update((Pixel *)buffer.data());
	}

	void draw(const Image &img, math::Rect<uint16_t> from, math::Vec2<uint16_t> to);
	void clear(Pixel value);
	math::Vec2<uint16_t> dim() const;
	uint32_t area() const;

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
		(void)display;
		PixelToaster::Key::Code code = key; // TODO how does this work? operator Code()
		m_events.push({ { (Key::Code)code }, InputState::DOWN });
	}

    virtual void onKeyPressed(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		(void)display;
		PixelToaster::Key::Code code = key;
		m_events.push({ { (Key::Code)code }, InputState::PRESSED });
	}

    virtual void onKeyUp(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		(void)display;
		PixelToaster::Key::Code code = key;
		m_events.push({ { (Key::Code)code }, InputState::UP });
	}

    virtual void onMouseButtonDown(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) override {
		(void)display;
		(void)mouse;
		//inputs.push({ { (Key::Code)key.Code() }, InputState::DOWN });
	}

    virtual void onMouseButtonUp(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) override {
		(void)display;
		(void)mouse;
		//inputs.push({ { (Key::Code)key.Code() }, InputState::UP });
	}

    virtual void onMouseMove(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) {
		(void)display;
		(void)mouse;
	}

    virtual void onActivate(PixelToaster::DisplayInterface& display, bool active) {
		(void)display;
		(void)active;
	}

    virtual void onOpen(PixelToaster::DisplayInterface& display) {
		(void)display;
	}

    virtual bool onClose(PixelToaster::DisplayInterface& display) {
		(void)display;
		return true;
	}
};

class Render {
public:
};

}
}
