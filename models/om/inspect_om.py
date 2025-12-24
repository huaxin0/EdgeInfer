#!/usr/bin/env python3
"""验证昇腾 OM 模型输入输出信息"""
import acl
import sys

def inspect_model(model_path):
    # 初始化 ACL
    ret = acl.init(None)
    if ret != 0:
        print(f"❌ ACL 初始化失败: {ret}")
        return False

    # 设置设备
    ret = acl.rt.set_device(0)
    if ret != 0:
        print(f"❌ 设置设备失败: {ret}")
        acl.finalize()
        return False

    # 加载模型
    model_id, ret = acl.mdl.load_from_file(model_path)
    if ret != 0:
        print(f"❌ 加载模型失败: {ret}")
        acl.rt.reset_device(0)
        acl.finalize()
        return False

    print("=" * 60)
    print(f"✅ 模型加载成功: {model_path}")
    print(f"   Model ID: {model_id}")
    print("=" * 60)

    # 创建模型描述
    model_desc = acl.mdl.create_desc()
    ret = acl.mdl.get_desc(model_desc, model_id)
    if ret != 0:
        print(f"❌ 获取模型描述失败: {ret}")
        return False

    # 输入信息
    num_inputs = acl.mdl.get_num_inputs(model_desc)
    print(f"\n📥 输入数量: {num_inputs}")
    for i in range(num_inputs):
        dims = acl.mdl.get_input_dims(model_desc, i)
        size = acl.mdl.get_input_size_by_index(model_desc, i)
        print(f"   输入 {i}:")
        print(f"     - 维度: {dims.dims}")
        print(f"     - 大小: {size} bytes ({size/1024/1024:.2f} MB)")

    # 输出信息
    num_outputs = acl.mdl.get_num_outputs(model_desc)
    print(f"\n📤 输出数量: {num_outputs}")
    for i in range(num_outputs):
        dims = acl.mdl.get_output_dims(model_desc, i)
        size = acl.mdl.get_output_size_by_index(model_desc, i)
        print(f"   输出 {i}:")
        print(f"     - 维度: {dims.dims}")
        print(f"     - 大小: {size} bytes ({size/1024/1024:.2f} MB)")

    print("\n" + "=" * 60)

    # 清理
    acl.mdl.unload(model_id)
    acl.rt.reset_device(0)
    acl.finalize()

    return True

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("用法: python3 inspect_om.py <om模型路径>")
        sys.exit(1)

    model_path = sys.argv[1]
    ok = inspect_model(model_path)
    sys.exit(0 if ok else 1)
