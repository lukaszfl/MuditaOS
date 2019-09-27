/*
 * BoxLayout.hpp
 *
 *  Created on: 16 maj 2019
 *      Author: robert
 */

#ifndef GUI_WIDGETS_BOXLAYOUT_HPP_
#define GUI_WIDGETS_BOXLAYOUT_HPP_

#include <cstdint>

#include "Layout.hpp"
#include "Rect.hpp"

namespace gui {

class BoxLayout: public Rect, public Layout {
protected:

	struct BoxElement {
		BoxElement( Item* item ) : item{item}, noUpdate{ false } {};
		Item* item;
		bool noUpdate;
	};
public:
	BoxLayout();
	BoxLayout( Item* parent, const uint32_t& x, const uint32_t& y, const uint32_t& w, const uint32_t& h);
	~BoxLayout();

	//virtual methods from Item
	void setPosition( const short& x, const short& y ) override;
	void setSize( const short& w, const short& h ) override;
	bool addWidget( gui::Item* item ) override;
	bool removeWidget( Item* item ) override;
	std::list<DrawCommand*> buildDrawList() override;
	virtual void resizeItems();
};

class HBox : public BoxLayout {
public:
	HBox();
	HBox( Item* parent, const uint32_t& x, const uint32_t& y, const uint32_t& w, const uint32_t& h);
	~HBox() {};
	void resizeItems() override;
};

class VBox : public BoxLayout {
public:
	VBox();
	VBox( Item* parent, const uint32_t& x, const uint32_t& y, const uint32_t& w, const uint32_t& h);
	~VBox() {};
	void resizeItems() override;
};

} /* namespace gui */

#endif /* GUI_WIDGETS_BOXLAYOUT_HPP_ */
