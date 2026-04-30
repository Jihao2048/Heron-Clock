import os
import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image
import piexif

def remove_exif(image_path):
    """去除图片的EXIF信息"""
    try:
        # 打开图片
        img = Image.open(image_path)
        
        # 获取图片格式
        img_format = img.format
        
        # 如果是JPEG格式，使用piexif去除EXIF
        if img_format in ['JPEG', 'JPG']:
            try:
                # 移除EXIF数据
                piexif.remove(image_path)
                print(f"成功移除 {os.path.basename(image_path)} 的EXIF信息")
                return True
            except Exception as e:
                print(f"使用piexif移除失败，尝试其他方法: {e}")
                # 如果piexif失败，使用PIL重新保存
                save_without_exif(img, image_path, img_format)
                return True
        else:
            # 对于其他格式，使用PIL重新保存
            save_without_exif(img, image_path, img_format)
            return True
            
    except Exception as e:
        print(f"处理图片时出错: {e}")
        return False

def save_without_exif(img, image_path, img_format):
    """不包含EXIF信息保存图片"""
    # 创建一个新的图像（去除EXIF）
    data = list(img.getdata())
    img_without_exif = Image.new(img.mode, img.size)
    img_without_exif.putdata(data)
    
    # 保存图片（不包含EXIF信息）
    if img_format in ['JPEG', 'JPG']:
        img_without_exif.save(image_path, 'JPEG', quality=95)
    elif img_format == 'PNG':
        img_without_exif.save(image_path, 'PNG')
    elif img_format == 'WEBP':
        img_without_exif.save(image_path, 'WEBP', quality=95)
    elif img_format == 'BMP':
        img_without_exif.save(image_path, 'BMP')
    else:
        # 其他格式保持原样
        img_without_exif.save(image_path, img_format)
    
    print(f"成功处理 {os.path.basename(image_path)} 并去除EXIF信息")

def select_file():
    """打开文件选择对话框"""
    root = tk.Tk()
    root.withdraw()  # 隐藏主窗口
    
    # 设置文件类型过滤器
    file_types = [
        ("图片文件", "*.jpg *.jpeg *.png *.bmp *.gif *.tiff *.webp"),
        ("所有文件", "*.*")
    ]
    
    # 打开文件选择对话框
    file_path = filedialog.askopenfilename(
        title="选择要处理的图片",
        filetypes=file_types
    )
    
    return file_path

def main():
    """主函数"""
    # 显示提示信息
    print("=" * 50)
    print("图片EXIF信息去除工具")
    print("=" * 50)
    print("提示：处理后会直接覆盖原文件，请确保有备份！")
    print("=" * 50)
    
    # 选择文件
    file_path = select_file()
    
    if not file_path:
        print("未选择文件，程序退出")
        # 显示提示框
        root = tk.Tk()
        root.withdraw()
        messagebox.showinfo("提示", "未选择文件，程序退出")
        return
    
    print(f"已选择文件: {file_path}")
    
    # 确认处理
    root = tk.Tk()
    root.withdraw()
    confirm = messagebox.askyesno(
        "确认", 
        f"将处理文件:\n{os.path.basename(file_path)}\n\n处理后会直接覆盖原文件，确定继续吗？"
    )
    
    if not confirm:
        print("用户取消操作")
        return
    
    # 处理图片
    print("正在处理图片...")
    success = remove_exif(file_path)
    
    if success:
        print("处理完成！")
        messagebox.showinfo("成功", f"图片处理完成！\n文件已更新: {os.path.basename(file_path)}")
    else:
        print("处理失败！")
        messagebox.showerror("错误", "图片处理失败，请检查文件格式")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"程序运行出错: {e}")
        messagebox.showerror("错误", f"程序运行出错:\n{str(e)}")