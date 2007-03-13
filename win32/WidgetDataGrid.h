#ifndef WIDGETDATAGRID_H_
#define WIDGETDATAGRID_H_

template< class EventHandlerClass, class MessageMapPolicy >
class WidgetDataGrid : public SmartWin::WidgetDataGrid<EventHandlerClass, MessageMapPolicy> {
private:
	typedef SmartWin::WidgetDataGrid<EventHandlerClass, MessageMapPolicy> BaseType;
public:
	// Constructor Taking pointer to parent
	explicit WidgetDataGrid( SmartWin::Widget * parent ) : BaseType(parent) { }

	using BaseType::addRemoveListViewExtendedStyle;
	
	bool setColumnOrder(const std::vector<int>& columns) {
		return ::SendMessage(this->handle(), LVM_SETCOLUMNORDERARRAY, static_cast<WPARAM>(columns.size()), reinterpret_cast<LPARAM>(&columns[0])) > 0;
	}
	std::vector<int> getColumnOrder() {
		std::vector<int> ret(this->getColumnCount());
		if(!::SendMessage(this->handle(), LVM_GETCOLUMNORDERARRAY, static_cast<WPARAM>(ret.size()), reinterpret_cast<LPARAM>(&ret[0]))) {
			ret.clear();
		}
		return ret;
	}
private:

};
#endif /*WIDGETDATAGRID_H_*/
