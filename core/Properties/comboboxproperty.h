#ifndef COMBOBOXPROPERTY_H
#define COMBOBOXPROPERTY_H
#include "Properties/property.h"
class ComboBoxProperty : public Property {
    Q_OBJECT
    friend class SelfRef;
protected:
    ComboBoxProperty(const QString& name,
                     const QStringList &list);
public:
    bool SWT_isComboBoxProperty() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    const QStringList &getValueNames() {
        return mValueNames;
    }

    QString getCurrentValueName() {
        if(mCurrentValue >= mValueNames.count()) return "null";
        return mValueNames.at(mCurrentValue);
    }

    const int &getCurrentValue() {
        return mCurrentValue;
    }
    void setCurrentValue(const int &id) {
        if(mCurrentValue == id) return;
        mCurrentValue = id;
        emit valueChanged(id);
        prp_afterWholeInfluenceRangeChanged();
    }
signals:
    void valueChanged(int);
private:
    int mCurrentValue = 0;
    QStringList mValueNames;
};

#endif // COMBOBOXPROPERTY_H
