from setuptools import find_packages, setup

package_name = 'nu_id_pubsub'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='zhalgas',
    maintainer_email='zhalgas@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
        	'id_publisher = nu_id_pubsub.id_publisher:main',
        	'id_subscriber = nu_id_pubsub.id_subscriber:main',
        ],
    },
)
